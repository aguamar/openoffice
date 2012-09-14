/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/adjitem.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/bolnitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/cntritem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/emphitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/postitem.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>

#include "patattr.hxx"
#include "docpool.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "document.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "conditio.hxx"
#include "validat.hxx"
#include "scmod.hxx"
#include "fillinfo.hxx"

// STATIC DATA -----------------------------------------------------------

ScDocument* ScPatternAttr::pDoc = NULL;

// -----------------------------------------------------------------------

//!	move to some header file
inline long TwipsToHMM(long nTwips)	{ return (nTwips * 127 + 36) / 72; }
inline long HMMToTwips(long nHMM)	{ return (nHMM * 72 + 63) / 127; }

// -----------------------------------------------------------------------

ScPatternAttr::ScPatternAttr( SfxItemSet* pItemSet, const String& rStyleName )
	:	SfxSetItem	( ATTR_PATTERN, pItemSet ),
		pName		( new String( rStyleName ) ),
		pStyle		( NULL )
{
}

ScPatternAttr::ScPatternAttr( SfxItemSet* pItemSet, ScStyleSheet* pStyleSheet )
	:	SfxSetItem	( ATTR_PATTERN, pItemSet ),
		pName		( NULL ),
		pStyle		( pStyleSheet )
{
	if ( pStyleSheet )
		GetItemSet().SetParent( &pStyleSheet->GetItemSet() );
}

ScPatternAttr::ScPatternAttr( SfxItemPool* pItemPool )
	:	SfxSetItem	( ATTR_PATTERN, new SfxItemSet( *pItemPool, ATTR_PATTERN_START, ATTR_PATTERN_END ) ),
		pName		( NULL ),
		pStyle		( NULL )
{
}

ScPatternAttr::ScPatternAttr( const ScPatternAttr& rPatternAttr )
	:	SfxSetItem	( rPatternAttr ),
		pStyle		( rPatternAttr.pStyle )
{
	if (rPatternAttr.pName)
		pName = new String(*rPatternAttr.pName);
	else
		pName = NULL;
}

__EXPORT ScPatternAttr::~ScPatternAttr()
{
	delete pName;
}

SfxPoolItem* __EXPORT ScPatternAttr::Clone( SfxItemPool *pPool ) const
{
	ScPatternAttr* pPattern = new ScPatternAttr( GetItemSet().Clone(sal_True, pPool) );

	pPattern->pStyle = pStyle;
	pPattern->pName  = pName ? new String(*pName) : NULL;

	return pPattern;
}

inline int StrCmp( const String* pStr1, const String* pStr2 )
{
	return ( pStr1 ? ( pStr2 ? ( *pStr1 == *pStr2 ) : sal_False ) : ( pStr2 ? sal_False : sal_True ) );
}



int __EXPORT ScPatternAttr::operator==( const SfxPoolItem& rCmp ) const
{
    // #i62090# Use quick comparison between ScPatternAttr's ItemSets

   //optimize a comparing operation from 'memcmp' to 'hash compare' to improve xls loading performance, i120575
   //More quickly comparing method using hashkey.
   //the const_cast here will then only change the HashKey of that SfxItemSet, so it's safe.
	return ((const_cast<ScPatternAttr&>(*this)).GetItemSet().QuickCompare( (const_cast<ScPatternAttr&>(static_cast<const ScPatternAttr&>(rCmp))).GetItemSet())&&
		StrCmp( GetStyleName(), static_cast<const ScPatternAttr&>(rCmp).GetStyleName() ) );  
}

SfxPoolItem* __EXPORT ScPatternAttr::Create( SvStream& rStream, sal_uInt16 /* nVersion */ ) const
{
	String* pStr;
	sal_Bool	bHasStyle;
	short	eFamDummy;

	rStream >> bHasStyle;

	if ( bHasStyle )
	{
		pStr = new String;
		rStream.ReadByteString( *pStr, rStream.GetStreamCharSet() );
		rStream >> eFamDummy; // wg. altem Dateiformat
	}
	else
		pStr = new String( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );

	SfxItemSet *pNewSet = new SfxItemSet( *GetItemSet().GetPool(),
									   ATTR_PATTERN_START, ATTR_PATTERN_END );
	pNewSet->Load( rStream );

	ScPatternAttr* pPattern = new ScPatternAttr( pNewSet );

	pPattern->pName = pStr;

	return pPattern;
}

SvStream& __EXPORT ScPatternAttr::Store(SvStream& rStream, sal_uInt16 /* nItemVersion */) const
{
	rStream << (sal_Bool)sal_True;

	if ( pStyle )
		rStream.WriteByteString( pStyle->GetName(), rStream.GetStreamCharSet() );
	else if ( pName )					// wenn Style geloescht ist/war
		rStream.WriteByteString( *pName, rStream.GetStreamCharSet() );
	else
		rStream.WriteByteString( ScGlobal::GetRscString(STR_STYLENAME_STANDARD),
									rStream.GetStreamCharSet() );

	rStream << (short)SFX_STYLE_FAMILY_PARA;  // wg. altem Dateiformat

	GetItemSet().Store( rStream );

	return rStream;
}

SvxCellOrientation ScPatternAttr::GetCellOrientation( const SfxItemSet& rItemSet, const SfxItemSet* pCondSet )
{
    SvxCellOrientation eOrient = SVX_ORIENTATION_STANDARD;

    if( ((const SfxBoolItem&)GetItem( ATTR_STACKED, rItemSet, pCondSet )).GetValue() )
    {
        eOrient = SVX_ORIENTATION_STACKED;
    }
    else
    {
        sal_Int32 nAngle = ((const SfxInt32Item&)GetItem( ATTR_ROTATE_VALUE, rItemSet, pCondSet )).GetValue();
        if( nAngle == 9000 )
            eOrient = SVX_ORIENTATION_BOTTOMTOP;
        else if( nAngle == 27000 )
            eOrient = SVX_ORIENTATION_TOPBOTTOM;
    }

    return eOrient;
}

SvxCellOrientation ScPatternAttr::GetCellOrientation( const SfxItemSet* pCondSet ) const
{
    return GetCellOrientation( GetItemSet(), pCondSet );
}

void ScPatternAttr::GetFont(
        Font& rFont, const SfxItemSet& rItemSet, ScAutoFontColorMode eAutoMode,
        OutputDevice* pOutDev, const Fraction* pScale,
        const SfxItemSet* pCondSet, sal_uInt8 nScript,
        const Color* pBackConfigColor, const Color* pTextConfigColor )
{
	//	Items auslesen

	const SvxFontItem* pFontAttr;
	sal_uInt32 nFontHeight;
	FontWeight eWeight;
	FontItalic eItalic;
	FontUnderline eUnder;
	FontUnderline eOver;
	sal_Bool bWordLine;
	FontStrikeout eStrike;
	sal_Bool bOutline;
	sal_Bool bShadow;
	FontEmphasisMark eEmphasis;
	FontRelief eRelief;
	Color aColor;
	LanguageType eLang;

	sal_uInt16 nFontId, nHeightId, nWeightId, nPostureId, nLangId;
	if ( nScript == SCRIPTTYPE_ASIAN )
	{
		nFontId    = ATTR_CJK_FONT;
		nHeightId  = ATTR_CJK_FONT_HEIGHT;
		nWeightId  = ATTR_CJK_FONT_WEIGHT;
		nPostureId = ATTR_CJK_FONT_POSTURE;
		nLangId    = ATTR_CJK_FONT_LANGUAGE;
	}
	else if ( nScript == SCRIPTTYPE_COMPLEX )
	{
		nFontId    = ATTR_CTL_FONT;
		nHeightId  = ATTR_CTL_FONT_HEIGHT;
		nWeightId  = ATTR_CTL_FONT_WEIGHT;
		nPostureId = ATTR_CTL_FONT_POSTURE;
		nLangId    = ATTR_CTL_FONT_LANGUAGE;
	}
	else
	{
		nFontId    = ATTR_FONT;
		nHeightId  = ATTR_FONT_HEIGHT;
		nWeightId  = ATTR_FONT_WEIGHT;
		nPostureId = ATTR_FONT_POSTURE;
		nLangId    = ATTR_FONT_LANGUAGE;
	}

	if ( pCondSet )
	{
		const SfxPoolItem* pItem;

		if ( pCondSet->GetItemState( nFontId, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( nFontId );
		pFontAttr = (const SvxFontItem*) pItem;

		if ( pCondSet->GetItemState( nHeightId, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( nHeightId );
		nFontHeight = ((const SvxFontHeightItem*)pItem)->GetHeight();

		if ( pCondSet->GetItemState( nWeightId, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( nWeightId );
		eWeight = (FontWeight)((const SvxWeightItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( nPostureId, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( nPostureId );
		eItalic = (FontItalic)((const SvxPostureItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_UNDERLINE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( ATTR_FONT_UNDERLINE );
		eUnder = (FontUnderline)((const SvxUnderlineItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_OVERLINE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( ATTR_FONT_OVERLINE );
		eOver = (FontUnderline)((const SvxOverlineItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_WORDLINE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( ATTR_FONT_WORDLINE );
		bWordLine = ((const SvxWordLineModeItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_CROSSEDOUT, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( ATTR_FONT_CROSSEDOUT );
		eStrike = (FontStrikeout)((const SvxCrossedOutItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_CONTOUR, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( ATTR_FONT_CONTOUR );
		bOutline = ((const SvxContourItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_SHADOWED, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( ATTR_FONT_SHADOWED );
		bShadow = ((const SvxShadowedItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_EMPHASISMARK, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( ATTR_FONT_EMPHASISMARK );
		eEmphasis = ((const SvxEmphasisMarkItem*)pItem)->GetEmphasisMark();

		if ( pCondSet->GetItemState( ATTR_FONT_RELIEF, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( ATTR_FONT_RELIEF );
		eRelief = (FontRelief)((const SvxCharReliefItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_COLOR, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rItemSet.Get( ATTR_FONT_COLOR );
		aColor = ((const SvxColorItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( nLangId, sal_True, &pItem ) != SFX_ITEM_SET )
			pItem = &rItemSet.Get( nLangId );
		eLang = ((const SvxLanguageItem*)pItem)->GetLanguage();
	}
    else    // alles aus rItemSet
	{
        pFontAttr = &(const SvxFontItem&)rItemSet.Get( nFontId );
		nFontHeight = ((const SvxFontHeightItem&)
                        rItemSet.Get( nHeightId )).GetHeight();
		eWeight = (FontWeight)((const SvxWeightItem&)
                        rItemSet.Get( nWeightId )).GetValue();
		eItalic = (FontItalic)((const SvxPostureItem&)
                        rItemSet.Get( nPostureId )).GetValue();
		eUnder = (FontUnderline)((const SvxUnderlineItem&)
                        rItemSet.Get( ATTR_FONT_UNDERLINE )).GetValue();
		eOver = (FontUnderline)((const SvxOverlineItem&)
                        rItemSet.Get( ATTR_FONT_OVERLINE )).GetValue();
		bWordLine = ((const SvxWordLineModeItem&)
                        rItemSet.Get( ATTR_FONT_WORDLINE )).GetValue();
		eStrike = (FontStrikeout)((const SvxCrossedOutItem&)
                        rItemSet.Get( ATTR_FONT_CROSSEDOUT )).GetValue();
		bOutline = ((const SvxContourItem&)
                        rItemSet.Get( ATTR_FONT_CONTOUR )).GetValue();
		bShadow = ((const SvxShadowedItem&)
                        rItemSet.Get( ATTR_FONT_SHADOWED )).GetValue();
		eEmphasis = ((const SvxEmphasisMarkItem&)
                        rItemSet.Get( ATTR_FONT_EMPHASISMARK )).GetEmphasisMark();
		eRelief = (FontRelief)((const SvxCharReliefItem&)
                        rItemSet.Get( ATTR_FONT_RELIEF )).GetValue();
		aColor = ((const SvxColorItem&)
                        rItemSet.Get( ATTR_FONT_COLOR )).GetValue();
		// for graphite language features
		eLang =
		((const SvxLanguageItem&)rItemSet.Get( nLangId )).GetLanguage();
	}
	DBG_ASSERT(pFontAttr,"nanu?");

	//	auswerten

	//	FontItem:

	if (rFont.GetName() != pFontAttr->GetFamilyName())
		rFont.SetName( pFontAttr->GetFamilyName() );
	if (rFont.GetStyleName() != pFontAttr->GetStyleName())
		rFont.SetStyleName( pFontAttr->GetStyleName() );

	rFont.SetFamily( pFontAttr->GetFamily() );
	rFont.SetCharSet( pFontAttr->GetCharSet() );
	rFont.SetPitch( pFontAttr->GetPitch() );

	rFont.SetLanguage(eLang);

	//	Groesse

	if ( pOutDev != NULL )
	{
		Size aEffSize;
		Fraction aFraction( 1,1 );
		if (pScale)
			aFraction = *pScale;
		Size aSize( 0, (long) nFontHeight );
		MapMode aDestMode = pOutDev->GetMapMode();
		MapMode aSrcMode( MAP_TWIP, Point(), aFraction, aFraction );
		if (aDestMode.GetMapUnit() == MAP_PIXEL)
			aEffSize = pOutDev->LogicToPixel( aSize, aSrcMode );
		else
		{
			Fraction aFractOne(1,1);
			aDestMode.SetScaleX( aFractOne );
			aDestMode.SetScaleY( aFractOne );
			aEffSize = OutputDevice::LogicToLogic( aSize, aSrcMode, aDestMode );
		}
		rFont.SetSize( aEffSize );
	}
	else /* if pOutDev != NULL */
	{
		rFont.SetSize( Size( 0, (long) nFontHeight ) );
	}

	//	determine effective font color

	if ( ( aColor.GetColor() == COL_AUTO && eAutoMode != SC_AUTOCOL_RAW ) ||
			eAutoMode == SC_AUTOCOL_IGNOREFONT || eAutoMode == SC_AUTOCOL_IGNOREALL )
	{
		if ( eAutoMode == SC_AUTOCOL_BLACK )
			aColor.SetColor( COL_BLACK );
		else
		{
			//	get background color from conditional or own set
			Color aBackColor;
			if ( pCondSet )
			{
				const SfxPoolItem* pItem;
				if ( pCondSet->GetItemState( ATTR_BACKGROUND, sal_True, &pItem ) != SFX_ITEM_SET )
                    pItem = &rItemSet.Get( ATTR_BACKGROUND );
				aBackColor = ((const SvxBrushItem*)pItem)->GetColor();
			}
			else
                aBackColor = ((const SvxBrushItem&)rItemSet.Get( ATTR_BACKGROUND )).GetColor();

			//	if background color attribute is transparent, use window color for brightness comparisons
			if ( aBackColor == COL_TRANSPARENT ||
					eAutoMode == SC_AUTOCOL_IGNOREBACK || eAutoMode == SC_AUTOCOL_IGNOREALL )
			{
				if ( eAutoMode == SC_AUTOCOL_PRINT )
					aBackColor.SetColor( COL_WHITE );
				else if ( pBackConfigColor )
				{
					// pBackConfigColor can be used to avoid repeated lookup of the configured color
					aBackColor = *pBackConfigColor;
				}
				else
                    aBackColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor );
			}

			//	get system text color for comparison
			Color aSysTextColor;
			if ( eAutoMode == SC_AUTOCOL_PRINT )
				aSysTextColor.SetColor( COL_BLACK );
			else if ( pTextConfigColor )
			{
				// pTextConfigColor can be used to avoid repeated lookup of the configured color
				aSysTextColor = *pTextConfigColor;
			}
			else
                aSysTextColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );

			//	select the resulting color
			if ( aBackColor.IsDark() && aSysTextColor.IsDark() )
			{
				//	use white instead of dark on dark
				aColor.SetColor( COL_WHITE );
			}
			else if ( aBackColor.IsBright() && aSysTextColor.IsBright() )
			{
				//	use black instead of bright on bright
				aColor.SetColor( COL_BLACK );
			}
			else
			{
				//	use aSysTextColor (black for SC_AUTOCOL_PRINT, from style settings otherwise)
				aColor = aSysTextColor;
			}
		}
	}

	//	set font effects
	rFont.SetWeight( eWeight );
	rFont.SetItalic( eItalic );
	rFont.SetUnderline( eUnder );
	rFont.SetOverline( eOver );
	rFont.SetWordLineMode( bWordLine );
	rFont.SetStrikeout( eStrike );
	rFont.SetOutline( bOutline );
	rFont.SetShadow( bShadow );
	rFont.SetEmphasisMark( eEmphasis );
	rFont.SetRelief( eRelief );
	rFont.SetColor( aColor );
	rFont.SetTransparent( sal_True );
}

void ScPatternAttr::GetFont(
        Font& rFont, ScAutoFontColorMode eAutoMode,
        OutputDevice* pOutDev, const Fraction* pScale,
        const SfxItemSet* pCondSet, sal_uInt8 nScript,
        const Color* pBackConfigColor, const Color* pTextConfigColor ) const
{
    GetFont( rFont, GetItemSet(), eAutoMode, pOutDev, pScale, pCondSet, nScript, pBackConfigColor, pTextConfigColor );
}


void ScPatternAttr::FillToEditItemSet( SfxItemSet& rEditSet, const SfxItemSet& rSrcSet, const SfxItemSet* pCondSet )
{
	//	Items auslesen

	SvxColorItem	aColorItem(EE_CHAR_COLOR);				// use item as-is
	SvxFontItem		aFontItem(EE_CHAR_FONTINFO);			// use item as-is
	SvxFontItem		aCjkFontItem(EE_CHAR_FONTINFO_CJK);
	SvxFontItem		aCtlFontItem(EE_CHAR_FONTINFO_CTL);
	long			nTHeight, nCjkTHeight, nCtlTHeight;		// Twips
	FontWeight		eWeight, eCjkWeight, eCtlWeight;
	SvxUnderlineItem aUnderlineItem(UNDERLINE_NONE, EE_CHAR_UNDERLINE);
	SvxOverlineItem aOverlineItem(UNDERLINE_NONE, EE_CHAR_OVERLINE);
	sal_Bool			bWordLine;
	FontStrikeout	eStrike;
	FontItalic		eItalic, eCjkItalic, eCtlItalic;
	sal_Bool			bOutline;
	sal_Bool			bShadow;
	sal_Bool			bForbidden;
	FontEmphasisMark eEmphasis;
	FontRelief		eRelief;
	LanguageType	eLang, eCjkLang, eCtlLang;
	sal_Bool			bHyphenate;
	SvxFrameDirection eDirection;

	//!	additional parameter to control if language is needed?

	if ( pCondSet )
	{
		const SfxPoolItem* pItem;

		if ( pCondSet->GetItemState( ATTR_FONT_COLOR, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_COLOR );
		aColorItem = *(const SvxColorItem*)pItem;

		if ( pCondSet->GetItemState( ATTR_FONT, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT );
		aFontItem = *(const SvxFontItem*)pItem;
		if ( pCondSet->GetItemState( ATTR_CJK_FONT, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_CJK_FONT );
		aCjkFontItem = *(const SvxFontItem*)pItem;
		if ( pCondSet->GetItemState( ATTR_CTL_FONT, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_CTL_FONT );
		aCtlFontItem = *(const SvxFontItem*)pItem;

		if ( pCondSet->GetItemState( ATTR_FONT_HEIGHT, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_HEIGHT );
		nTHeight = ((const SvxFontHeightItem*)pItem)->GetHeight();
		if ( pCondSet->GetItemState( ATTR_CJK_FONT_HEIGHT, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_CJK_FONT_HEIGHT );
		nCjkTHeight = ((const SvxFontHeightItem*)pItem)->GetHeight();
		if ( pCondSet->GetItemState( ATTR_CTL_FONT_HEIGHT, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_CTL_FONT_HEIGHT );
		nCtlTHeight = ((const SvxFontHeightItem*)pItem)->GetHeight();

		if ( pCondSet->GetItemState( ATTR_FONT_WEIGHT, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_WEIGHT );
		eWeight = (FontWeight)((const SvxWeightItem*)pItem)->GetValue();
		if ( pCondSet->GetItemState( ATTR_CJK_FONT_WEIGHT, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_CJK_FONT_WEIGHT );
		eCjkWeight = (FontWeight)((const SvxWeightItem*)pItem)->GetValue();
		if ( pCondSet->GetItemState( ATTR_CTL_FONT_WEIGHT, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_CTL_FONT_WEIGHT );
		eCtlWeight = (FontWeight)((const SvxWeightItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_POSTURE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_POSTURE );
		eItalic = (FontItalic)((const SvxPostureItem*)pItem)->GetValue();
		if ( pCondSet->GetItemState( ATTR_CJK_FONT_POSTURE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_CJK_FONT_POSTURE );
		eCjkItalic = (FontItalic)((const SvxPostureItem*)pItem)->GetValue();
		if ( pCondSet->GetItemState( ATTR_CTL_FONT_POSTURE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_CTL_FONT_POSTURE );
		eCtlItalic = (FontItalic)((const SvxPostureItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_UNDERLINE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_UNDERLINE );
		aUnderlineItem = *(const SvxUnderlineItem*)pItem;

		if ( pCondSet->GetItemState( ATTR_FONT_OVERLINE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_OVERLINE );
		aOverlineItem = *(const SvxOverlineItem*)pItem;

		if ( pCondSet->GetItemState( ATTR_FONT_WORDLINE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_WORDLINE );
		bWordLine = ((const SvxWordLineModeItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_CROSSEDOUT, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_CROSSEDOUT );
		eStrike = (FontStrikeout)((const SvxCrossedOutItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_CONTOUR, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_CONTOUR );
		bOutline = ((const SvxContourItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_SHADOWED, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_SHADOWED );
		bShadow = ((const SvxShadowedItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FORBIDDEN_RULES, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FORBIDDEN_RULES );
		bForbidden = ((const SvxForbiddenRuleItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_EMPHASISMARK, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_EMPHASISMARK );
		eEmphasis = ((const SvxEmphasisMarkItem*)pItem)->GetEmphasisMark();
		if ( pCondSet->GetItemState( ATTR_FONT_RELIEF, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_RELIEF );
		eRelief = (FontRelief)((const SvxCharReliefItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_FONT_LANGUAGE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_FONT_LANGUAGE );
		eLang = ((const SvxLanguageItem*)pItem)->GetLanguage();
		if ( pCondSet->GetItemState( ATTR_CJK_FONT_LANGUAGE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_CJK_FONT_LANGUAGE );
		eCjkLang = ((const SvxLanguageItem*)pItem)->GetLanguage();
		if ( pCondSet->GetItemState( ATTR_CTL_FONT_LANGUAGE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_CTL_FONT_LANGUAGE );
		eCtlLang = ((const SvxLanguageItem*)pItem)->GetLanguage();

		if ( pCondSet->GetItemState( ATTR_HYPHENATE, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_HYPHENATE );
		bHyphenate = ((const SfxBoolItem*)pItem)->GetValue();

		if ( pCondSet->GetItemState( ATTR_WRITINGDIR, sal_True, &pItem ) != SFX_ITEM_SET )
            pItem = &rSrcSet.Get( ATTR_WRITINGDIR );
		eDirection = (SvxFrameDirection)((const SvxFrameDirectionItem*)pItem)->GetValue();
	}
	else		// alles direkt aus Pattern
	{
        aColorItem = (const SvxColorItem&) rSrcSet.Get( ATTR_FONT_COLOR );
        aFontItem = (const SvxFontItem&) rSrcSet.Get( ATTR_FONT );
        aCjkFontItem = (const SvxFontItem&) rSrcSet.Get( ATTR_CJK_FONT );
        aCtlFontItem = (const SvxFontItem&) rSrcSet.Get( ATTR_CTL_FONT );
		nTHeight = ((const SvxFontHeightItem&)
                        rSrcSet.Get( ATTR_FONT_HEIGHT )).GetHeight();
		nCjkTHeight = ((const SvxFontHeightItem&)
                        rSrcSet.Get( ATTR_CJK_FONT_HEIGHT )).GetHeight();
		nCtlTHeight = ((const SvxFontHeightItem&)
                        rSrcSet.Get( ATTR_CTL_FONT_HEIGHT )).GetHeight();
		eWeight = (FontWeight)((const SvxWeightItem&)
                        rSrcSet.Get( ATTR_FONT_WEIGHT )).GetValue();
		eCjkWeight = (FontWeight)((const SvxWeightItem&)
                        rSrcSet.Get( ATTR_CJK_FONT_WEIGHT )).GetValue();
		eCtlWeight = (FontWeight)((const SvxWeightItem&)
                        rSrcSet.Get( ATTR_CTL_FONT_WEIGHT )).GetValue();
		eItalic = (FontItalic)((const SvxPostureItem&)
                        rSrcSet.Get( ATTR_FONT_POSTURE )).GetValue();
		eCjkItalic = (FontItalic)((const SvxPostureItem&)
                        rSrcSet.Get( ATTR_CJK_FONT_POSTURE )).GetValue();
		eCtlItalic = (FontItalic)((const SvxPostureItem&)
                        rSrcSet.Get( ATTR_CTL_FONT_POSTURE )).GetValue();
        aUnderlineItem = (const SvxUnderlineItem&) rSrcSet.Get( ATTR_FONT_UNDERLINE );
        aOverlineItem  = (const SvxOverlineItem&) rSrcSet.Get( ATTR_FONT_OVERLINE );
		bWordLine = ((const SvxWordLineModeItem&)
                        rSrcSet.Get( ATTR_FONT_WORDLINE )).GetValue();
		eStrike = (FontStrikeout)((const SvxCrossedOutItem&)
                        rSrcSet.Get( ATTR_FONT_CROSSEDOUT )).GetValue();
		bOutline = ((const SvxContourItem&)
                        rSrcSet.Get( ATTR_FONT_CONTOUR )).GetValue();
		bShadow = ((const SvxShadowedItem&)
                        rSrcSet.Get( ATTR_FONT_SHADOWED )).GetValue();
		bForbidden = ((const SvxForbiddenRuleItem&)
                        rSrcSet.Get( ATTR_FORBIDDEN_RULES )).GetValue();
		eEmphasis = ((const SvxEmphasisMarkItem&)
                        rSrcSet.Get( ATTR_FONT_EMPHASISMARK )).GetEmphasisMark();
		eRelief = (FontRelief)((const SvxCharReliefItem&)
                        rSrcSet.Get( ATTR_FONT_RELIEF )).GetValue();
		eLang = ((const SvxLanguageItem&)
                        rSrcSet.Get( ATTR_FONT_LANGUAGE )).GetLanguage();
		eCjkLang = ((const SvxLanguageItem&)
                        rSrcSet.Get( ATTR_CJK_FONT_LANGUAGE )).GetLanguage();
		eCtlLang = ((const SvxLanguageItem&)
                        rSrcSet.Get( ATTR_CTL_FONT_LANGUAGE )).GetLanguage();
		bHyphenate = ((const SfxBoolItem&)
                        rSrcSet.Get( ATTR_HYPHENATE )).GetValue();
		eDirection = (SvxFrameDirection)((const SvxFrameDirectionItem&)
                        rSrcSet.Get( ATTR_WRITINGDIR )).GetValue();
	}

	// kompatibel zu LogicToLogic rechnen, also 2540/1440 = 127/72, und runden

	long nHeight = TwipsToHMM(nTHeight);
	long nCjkHeight = TwipsToHMM(nCjkTHeight);
	long nCtlHeight = TwipsToHMM(nCtlTHeight);

	//	put items into EditEngine ItemSet

	if ( aColorItem.GetValue().GetColor() == COL_AUTO )
	{
		//	#108979# When cell attributes are converted to EditEngine paragraph attributes,
		//	don't create a hard item for automatic color, because that would be converted
		//	to black when the item's Store method is used in CreateTransferable/WriteBin.
		//	COL_AUTO is the EditEngine's pool default, so ClearItem will result in automatic
		//	color, too, without having to store the item.
		rEditSet.ClearItem( EE_CHAR_COLOR );
	}
	else
	    rEditSet.Put( aColorItem );
    rEditSet.Put( aFontItem );
    rEditSet.Put( aCjkFontItem );
    rEditSet.Put( aCtlFontItem );
    rEditSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT ) );
    rEditSet.Put( SvxFontHeightItem( nCjkHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
    rEditSet.Put( SvxFontHeightItem( nCtlHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );
    rEditSet.Put( SvxWeightItem ( eWeight,      EE_CHAR_WEIGHT ) );
    rEditSet.Put( SvxWeightItem ( eCjkWeight,   EE_CHAR_WEIGHT_CJK ) );
    rEditSet.Put( SvxWeightItem ( eCtlWeight,   EE_CHAR_WEIGHT_CTL ) );
    rEditSet.Put( aUnderlineItem );
    rEditSet.Put( aOverlineItem );
    rEditSet.Put( SvxWordLineModeItem( bWordLine,   EE_CHAR_WLM ) );
    rEditSet.Put( SvxCrossedOutItem( eStrike,       EE_CHAR_STRIKEOUT ) );
    rEditSet.Put( SvxPostureItem    ( eItalic,      EE_CHAR_ITALIC ) );
    rEditSet.Put( SvxPostureItem    ( eCjkItalic,   EE_CHAR_ITALIC_CJK ) );
    rEditSet.Put( SvxPostureItem    ( eCtlItalic,   EE_CHAR_ITALIC_CTL ) );
    rEditSet.Put( SvxContourItem    ( bOutline,     EE_CHAR_OUTLINE ) );
    rEditSet.Put( SvxShadowedItem   ( bShadow,      EE_CHAR_SHADOW ) );
    rEditSet.Put( SfxBoolItem       ( EE_PARA_FORBIDDENRULES, bForbidden ) );
    rEditSet.Put( SvxEmphasisMarkItem( eEmphasis,   EE_CHAR_EMPHASISMARK ) );
    rEditSet.Put( SvxCharReliefItem( eRelief,       EE_CHAR_RELIEF ) );
    rEditSet.Put( SvxLanguageItem   ( eLang,        EE_CHAR_LANGUAGE ) );
    rEditSet.Put( SvxLanguageItem   ( eCjkLang,     EE_CHAR_LANGUAGE_CJK ) );
    rEditSet.Put( SvxLanguageItem   ( eCtlLang,     EE_CHAR_LANGUAGE_CTL ) );
    rEditSet.Put( SfxBoolItem       ( EE_PARA_HYPHENATE, bHyphenate ) );
    rEditSet.Put( SvxFrameDirectionItem( eDirection, EE_PARA_WRITINGDIR ) );

    // #111216# Script spacing is always off.
    // The cell attribute isn't used here as long as there is no UI to set it
    // (don't evaluate attributes that can't be changed).
    // If a locale-dependent default is needed, it has to go into the cell
    // style, like the fonts.
    rEditSet.Put( SvxScriptSpaceItem( sal_False, EE_PARA_ASIANCJKSPACING ) );
}

void ScPatternAttr::FillEditItemSet( SfxItemSet* pEditSet, const SfxItemSet* pCondSet ) const
{
    if( pEditSet )
        FillToEditItemSet( *pEditSet, GetItemSet(), pCondSet );
}


void ScPatternAttr::GetFromEditItemSet( SfxItemSet& rDestSet, const SfxItemSet& rEditSet )
{
	const SfxPoolItem* pItem;

    if (rEditSet.GetItemState(EE_CHAR_COLOR,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxColorItem(ATTR_FONT_COLOR) = *(const SvxColorItem*)pItem );

    if (rEditSet.GetItemState(EE_CHAR_FONTINFO,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxFontItem(ATTR_FONT) = *(const SvxFontItem*)pItem );
    if (rEditSet.GetItemState(EE_CHAR_FONTINFO_CJK,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxFontItem(ATTR_CJK_FONT) = *(const SvxFontItem*)pItem );
    if (rEditSet.GetItemState(EE_CHAR_FONTINFO_CTL,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxFontItem(ATTR_CTL_FONT) = *(const SvxFontItem*)pItem );

    if (rEditSet.GetItemState(EE_CHAR_FONTHEIGHT,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxFontHeightItem( HMMToTwips( ((const SvxFontHeightItem*)pItem)->GetHeight() ),
						100, ATTR_FONT_HEIGHT ) );
    if (rEditSet.GetItemState(EE_CHAR_FONTHEIGHT_CJK,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxFontHeightItem( HMMToTwips( ((const SvxFontHeightItem*)pItem)->GetHeight() ),
						100, ATTR_CJK_FONT_HEIGHT ) );
    if (rEditSet.GetItemState(EE_CHAR_FONTHEIGHT_CTL,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxFontHeightItem( HMMToTwips( ((const SvxFontHeightItem*)pItem)->GetHeight() ),
						100, ATTR_CTL_FONT_HEIGHT ) );

    if (rEditSet.GetItemState(EE_CHAR_WEIGHT,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxWeightItem( (FontWeight)((const SvxWeightItem*)pItem)->GetValue(),
						ATTR_FONT_WEIGHT) );
    if (rEditSet.GetItemState(EE_CHAR_WEIGHT_CJK,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxWeightItem( (FontWeight)((const SvxWeightItem*)pItem)->GetValue(),
						ATTR_CJK_FONT_WEIGHT) );
    if (rEditSet.GetItemState(EE_CHAR_WEIGHT_CTL,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxWeightItem( (FontWeight)((const SvxWeightItem*)pItem)->GetValue(),
						ATTR_CTL_FONT_WEIGHT) );

	// SvxTextLineItem contains enum and color
    if (rEditSet.GetItemState(EE_CHAR_UNDERLINE,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxUnderlineItem(UNDERLINE_NONE,ATTR_FONT_UNDERLINE) = *(const SvxUnderlineItem*)pItem );
    if (rEditSet.GetItemState(EE_CHAR_OVERLINE,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxOverlineItem(UNDERLINE_NONE,ATTR_FONT_OVERLINE) = *(const SvxOverlineItem*)pItem );
    if (rEditSet.GetItemState(EE_CHAR_WLM,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxWordLineModeItem( ((const SvxWordLineModeItem*)pItem)->GetValue(),
						ATTR_FONT_WORDLINE) );

    if (rEditSet.GetItemState(EE_CHAR_STRIKEOUT,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxCrossedOutItem( (FontStrikeout)((const SvxCrossedOutItem*)pItem)->GetValue(),
						ATTR_FONT_CROSSEDOUT) );

    if (rEditSet.GetItemState(EE_CHAR_ITALIC,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxPostureItem( (FontItalic)((const SvxPostureItem*)pItem)->GetValue(),
						ATTR_FONT_POSTURE) );
    if (rEditSet.GetItemState(EE_CHAR_ITALIC_CJK,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxPostureItem( (FontItalic)((const SvxPostureItem*)pItem)->GetValue(),
						ATTR_CJK_FONT_POSTURE) );
    if (rEditSet.GetItemState(EE_CHAR_ITALIC_CTL,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxPostureItem( (FontItalic)((const SvxPostureItem*)pItem)->GetValue(),
						ATTR_CTL_FONT_POSTURE) );

    if (rEditSet.GetItemState(EE_CHAR_OUTLINE,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxContourItem( ((const SvxContourItem*)pItem)->GetValue(),
						ATTR_FONT_CONTOUR) );
    if (rEditSet.GetItemState(EE_CHAR_SHADOW,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxShadowedItem( ((const SvxShadowedItem*)pItem)->GetValue(),
						ATTR_FONT_SHADOWED) );
    if (rEditSet.GetItemState(EE_CHAR_EMPHASISMARK,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxEmphasisMarkItem( ((const SvxEmphasisMarkItem*)pItem)->GetEmphasisMark(),
						ATTR_FONT_EMPHASISMARK) );
    if (rEditSet.GetItemState(EE_CHAR_RELIEF,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxCharReliefItem( (FontRelief)((const SvxCharReliefItem*)pItem)->GetValue(),
						ATTR_FONT_RELIEF) );

    if (rEditSet.GetItemState(EE_CHAR_LANGUAGE,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxLanguageItem(static_cast<const SvxLanguageItem*>(pItem)->GetValue(), ATTR_FONT_LANGUAGE) );
    if (rEditSet.GetItemState(EE_CHAR_LANGUAGE_CJK,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxLanguageItem(static_cast<const SvxLanguageItem*>(pItem)->GetValue(), ATTR_CJK_FONT_LANGUAGE) );
    if (rEditSet.GetItemState(EE_CHAR_LANGUAGE_CTL,sal_True,&pItem) == SFX_ITEM_SET)
        rDestSet.Put( SvxLanguageItem(static_cast<const SvxLanguageItem*>(pItem)->GetValue(), ATTR_CTL_FONT_LANGUAGE) );

    if (rEditSet.GetItemState(EE_PARA_JUST,sal_True,&pItem) == SFX_ITEM_SET)
	{
		SvxCellHorJustify eVal;
		switch ( ((const SvxAdjustItem*)pItem)->GetAdjust() )
		{
			case SVX_ADJUST_LEFT:
				// #30154# EditEngine Default ist bei dem GetAttribs() ItemSet
				// immer gesetzt!
				// ob links oder rechts entscheiden wir selbst bei Text/Zahl
				eVal = SVX_HOR_JUSTIFY_STANDARD;
				break;
			case SVX_ADJUST_RIGHT:
				eVal = SVX_HOR_JUSTIFY_RIGHT;
				break;
			case SVX_ADJUST_BLOCK:
				eVal = SVX_HOR_JUSTIFY_BLOCK;
				break;
			case SVX_ADJUST_CENTER:
				eVal = SVX_HOR_JUSTIFY_CENTER;
				break;
			case SVX_ADJUST_BLOCKLINE:
				eVal = SVX_HOR_JUSTIFY_BLOCK;
				break;
			case SVX_ADJUST_END:
				eVal = SVX_HOR_JUSTIFY_RIGHT;
				break;
			default:
				eVal = SVX_HOR_JUSTIFY_STANDARD;
		}
		if ( eVal != SVX_HOR_JUSTIFY_STANDARD )
            rDestSet.Put( SvxHorJustifyItem( eVal, ATTR_HOR_JUSTIFY) );
	}
}

void ScPatternAttr::GetFromEditItemSet( const SfxItemSet* pEditSet )
{
    if( pEditSet )
        GetFromEditItemSet( GetItemSet(), *pEditSet );
}

void ScPatternAttr::FillEditParaItems( SfxItemSet* pEditSet ) const
{
	//	in GetFromEditItemSet schon dabei, in FillEditItemSet aber nicht
	//	Hor. Ausrichtung Standard wird immer als "links" umgesetzt

	const SfxItemSet& rMySet = GetItemSet();

	SvxCellHorJustify eHorJust = (SvxCellHorJustify)
		((const SvxHorJustifyItem&)rMySet.Get(ATTR_HOR_JUSTIFY)).GetValue();

	SvxAdjust eSvxAdjust;
	switch (eHorJust)
	{
		case SVX_HOR_JUSTIFY_RIGHT:	 eSvxAdjust = SVX_ADJUST_RIGHT;	 break;
		case SVX_HOR_JUSTIFY_CENTER: eSvxAdjust = SVX_ADJUST_CENTER; break;
		case SVX_HOR_JUSTIFY_BLOCK:	 eSvxAdjust = SVX_ADJUST_BLOCK;	 break;
		default:					 eSvxAdjust = SVX_ADJUST_LEFT;	 break;
	}
	pEditSet->Put( SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );
}

void ScPatternAttr::DeleteUnchanged( const ScPatternAttr* pOldAttrs )
{
	SfxItemSet& rThisSet = GetItemSet();
	const SfxItemSet& rOldSet = pOldAttrs->GetItemSet();

	const SfxPoolItem* pThisItem;
	const SfxPoolItem* pOldItem;

    for ( sal_uInt16 nSubWhich=ATTR_PATTERN_START; nSubWhich<=ATTR_PATTERN_END; nSubWhich++ )
	{
		//	only items that are set are interesting
        if ( rThisSet.GetItemState( nSubWhich, sal_False, &pThisItem ) == SFX_ITEM_SET )
        {
            SfxItemState eOldState = rOldSet.GetItemState( nSubWhich, sal_True, &pOldItem );
            if ( eOldState == SFX_ITEM_SET )
			{
				//	item is set in OldAttrs (or its parent) -> compare pointers
				if ( pThisItem == pOldItem )
                    rThisSet.ClearItem( nSubWhich );
			}
            else if ( eOldState != SFX_ITEM_DONTCARE )
			{
				//	not set in OldAttrs -> compare item value to default item
                if ( *pThisItem == rThisSet.GetPool()->GetDefaultItem( nSubWhich ) )
                    rThisSet.ClearItem( nSubWhich );
			}
		}
	}
}

sal_Bool ScPatternAttr::HasItemsSet( const sal_uInt16* pWhich ) const
{
	const SfxItemSet& rSet = GetItemSet();
	for (sal_uInt16 i=0; pWhich[i]; i++)
		if ( rSet.GetItemState( pWhich[i], sal_False ) == SFX_ITEM_SET )
			return sal_True;
	return sal_False;
}

void ScPatternAttr::ClearItems( const sal_uInt16* pWhich )
{
	SfxItemSet& rSet = GetItemSet();
	for (sal_uInt16 i=0; pWhich[i]; i++)
		rSet.ClearItem(pWhich[i]);
}

SfxStyleSheetBase* lcl_CopyStyleToPool
	(
		SfxStyleSheetBase*		pSrcStyle,
		SfxStyleSheetBasePool*	pSrcPool,
        SfxStyleSheetBasePool*	pDestPool,
        const SvNumberFormatterIndexTable*     pFormatExchangeList
	)
{
	if ( !pSrcStyle || !pDestPool || !pSrcPool )
	{
		DBG_ERROR( "CopyStyleToPool: Invalid Arguments :-/" );
		return NULL;
	}

	//--------------------------------------------------------

	const String		 aStrSrcStyle = pSrcStyle->GetName();
	const SfxStyleFamily eFamily	  = pSrcStyle->GetFamily();
	SfxStyleSheetBase*	 pDestStyle   = pDestPool->Find( aStrSrcStyle, eFamily );

	if ( !pDestStyle )
	{
		const String  aStrParent = pSrcStyle->GetParent();
        const SfxItemSet& rSrcSet = pSrcStyle->GetItemSet();

		pDestStyle = &pDestPool->Make( aStrSrcStyle, eFamily, SFXSTYLEBIT_USERDEF );
        SfxItemSet& rDestSet = pDestStyle->GetItemSet();
        rDestSet.Put( rSrcSet );

        // #b5017505# number format exchange list has to be handled here, too
        // (only called for cell styles)

        const SfxPoolItem* pSrcItem;
        if ( pFormatExchangeList &&
             rSrcSet.GetItemState( ATTR_VALUE_FORMAT, sal_False, &pSrcItem ) == SFX_ITEM_SET )
        {
            sal_uLong nOldFormat = static_cast<const SfxUInt32Item*>(pSrcItem)->GetValue();
            sal_uInt32* pNewFormat = static_cast<sal_uInt32*>(pFormatExchangeList->Get( nOldFormat ));
            if (pNewFormat)
                rDestSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, *pNewFormat ) );
        }

		// ggF. abgeleitete Styles erzeugen, wenn nicht vorhanden:

		if ( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) != aStrParent &&
			 aStrSrcStyle != aStrParent &&
			 !pDestPool->Find( aStrParent, eFamily ) )
		{
			lcl_CopyStyleToPool( pSrcPool->Find( aStrParent, eFamily ),
								 pSrcPool, pDestPool, pFormatExchangeList );
		}

		pDestStyle->SetParent( aStrParent );
	}

	return pDestStyle;
}

ScPatternAttr* ScPatternAttr::PutInPool( ScDocument* pDestDoc, ScDocument* pSrcDoc ) const
{
	const SfxItemSet* pSrcSet = &GetItemSet();

	ScPatternAttr* pDestPattern = new ScPatternAttr(pDestDoc->GetPool());
	SfxItemSet* pDestSet = &pDestPattern->GetItemSet();

	// Zellformatvorlage in anderes Dokument kopieren:

	if ( pDestDoc != pSrcDoc )
	{
		DBG_ASSERT( pStyle, "Missing Pattern-Style! :-/" );

		// wenn Vorlage im DestDoc vorhanden, dieses benutzen, sonst Style
		// mit Parent-Vorlagen kopieren/ggF. erzeugen und dem DestDoc hinzufuegen

		SfxStyleSheetBase* pStyleCpy = lcl_CopyStyleToPool( pStyle,
															pSrcDoc->GetStyleSheetPool(),
															pDestDoc->GetStyleSheetPool(),
															pDestDoc->GetFormatExchangeList() );

		pDestPattern->SetStyleSheet( (ScStyleSheet*)pStyleCpy );
	}

	for ( sal_uInt16 nAttrId = ATTR_PATTERN_START; nAttrId <= ATTR_PATTERN_END; nAttrId++ )
	{
		const SfxPoolItem* pSrcItem;
		SfxItemState eItemState = pSrcSet->GetItemState( nAttrId, sal_False, &pSrcItem );
		if (eItemState==SFX_ITEM_ON)
		{
			SfxPoolItem* pNewItem = NULL;

			if ( nAttrId == ATTR_CONDITIONAL )
			{
				//	Bedingte Formate ins neue Dokument kopieren

				sal_uLong nNewIndex = 0;
				ScConditionalFormatList* pSrcList = pSrcDoc->GetCondFormList();
				if ( pSrcList )
				{
					sal_uLong nOldIndex = ((const SfxUInt32Item*)pSrcItem)->GetValue();
					const ScConditionalFormat* pOldData = pSrcList->GetFormat( nOldIndex );
					if ( pOldData )
					{
						nNewIndex = pDestDoc->AddCondFormat( *pOldData );

						//	zugehoerige Styles auch mitkopieren
						//!	nur wenn Format bei Add neu angelegt

						ScStyleSheetPool* pSrcSPool = pSrcDoc->GetStyleSheetPool();
						ScStyleSheetPool* pDestSPool = pDestDoc->GetStyleSheetPool();
                        const SvNumberFormatterIndexTable* pFormatExchangeList = pDestDoc->GetFormatExchangeList();
						sal_uInt16 nStlCnt = pOldData->Count();
						for (sal_uInt16 i=0; i<nStlCnt; i++)
						{
							String aName = pOldData->GetEntry(i)->GetStyle();
							SfxStyleSheetBase* pSrcStl =
								pSrcDoc->GetStyleSheetPool()->Find(aName, SFX_STYLE_FAMILY_PARA);
							lcl_CopyStyleToPool( pSrcStl, pSrcSPool, pDestSPool, pFormatExchangeList );
						}
					}
				}
				pNewItem = new SfxUInt32Item( ATTR_CONDITIONAL, nNewIndex );
			}
			else if ( nAttrId == ATTR_VALIDDATA )
			{
				//	Gueltigkeit ins neue Dokument kopieren

				sal_uLong nNewIndex = 0;
				ScValidationDataList* pSrcList = pSrcDoc->GetValidationList();
				if ( pSrcList )
				{
					sal_uLong nOldIndex = ((const SfxUInt32Item*)pSrcItem)->GetValue();
					const ScValidationData* pOldData = pSrcList->GetData( nOldIndex );
					if ( pOldData )
						nNewIndex = pDestDoc->AddValidationEntry( *pOldData );
				}
				pNewItem = new SfxUInt32Item( ATTR_VALIDDATA, nNewIndex );
			}
			else if ( nAttrId == ATTR_VALUE_FORMAT && pDestDoc->GetFormatExchangeList() )
			{
				//	Zahlformate nach Exchange-Liste

				sal_uLong nOldFormat = ((const SfxUInt32Item*)pSrcItem)->GetValue();
				sal_uInt32* pNewFormat = static_cast<sal_uInt32*>(pDestDoc->GetFormatExchangeList()->Get(nOldFormat));
				if (pNewFormat)
					pNewItem = new SfxUInt32Item( ATTR_VALUE_FORMAT, (sal_uInt32) (*pNewFormat) );
			}

			if ( pNewItem )
			{
				pDestSet->Put(*pNewItem);
				delete pNewItem;
			}
			else
				pDestSet->Put(*pSrcItem);
		}
	}

	ScPatternAttr* pPatternAttr =
		(ScPatternAttr*) &pDestDoc->GetPool()->Put(*pDestPattern);
	delete pDestPattern;
	return pPatternAttr;
}

sal_Bool ScPatternAttr::IsVisible() const
{
	const SfxItemSet& rSet = GetItemSet();

	const SfxPoolItem* pItem;
	SfxItemState eState;

	eState = rSet.GetItemState( ATTR_BACKGROUND, sal_True, &pItem );
	if ( eState == SFX_ITEM_SET )
        if ( ((const SvxBrushItem*)pItem)->GetColor().GetColor() != COL_TRANSPARENT )
			return sal_True;

	eState = rSet.GetItemState( ATTR_BORDER, sal_True, &pItem );
	if ( eState == SFX_ITEM_SET )
	{
        const SvxBoxItem* pBoxItem = (SvxBoxItem*) pItem;
		if ( pBoxItem->GetTop() || pBoxItem->GetBottom() ||
			 pBoxItem->GetLeft() || pBoxItem->GetRight() )
			return sal_True;
	}

    eState = rSet.GetItemState( ATTR_BORDER_TLBR, sal_True, &pItem );
    if ( eState == SFX_ITEM_SET )
        if( static_cast< const SvxLineItem* >( pItem )->GetLine() )
            return sal_True;

    eState = rSet.GetItemState( ATTR_BORDER_BLTR, sal_True, &pItem );
    if ( eState == SFX_ITEM_SET )
        if( static_cast< const SvxLineItem* >( pItem )->GetLine() )
            return sal_True;

	eState = rSet.GetItemState( ATTR_SHADOW, sal_True, &pItem );
	if ( eState == SFX_ITEM_SET )
        if ( ((const SvxShadowItem*)pItem)->GetLocation() != SVX_SHADOW_NONE )
			return sal_True;

	return sal_False;
}

inline sal_Bool OneEqual( const SfxItemSet& rSet1, const SfxItemSet& rSet2, sal_uInt16 nId )
{
	const SfxPoolItem* pItem1 = &rSet1.Get(nId);
	const SfxPoolItem* pItem2 = &rSet2.Get(nId);
	return ( pItem1 == pItem2 || *pItem1 == *pItem2 );
}

sal_Bool ScPatternAttr::IsVisibleEqual( const ScPatternAttr& rOther ) const
{
	const SfxItemSet& rThisSet = GetItemSet();
	const SfxItemSet& rOtherSet = rOther.GetItemSet();

	return OneEqual( rThisSet, rOtherSet, ATTR_BACKGROUND ) &&
			OneEqual( rThisSet, rOtherSet, ATTR_BORDER ) &&
            OneEqual( rThisSet, rOtherSet, ATTR_BORDER_TLBR ) &&
            OneEqual( rThisSet, rOtherSet, ATTR_BORDER_BLTR ) &&
			OneEqual( rThisSet, rOtherSet, ATTR_SHADOW );

	//!		auch hier nur wirklich sichtbare Werte testen !!!
}

const String* ScPatternAttr::GetStyleName() const
{
	return pName ? pName : ( pStyle ? &pStyle->GetName() : NULL );
}


void ScPatternAttr::SetStyleSheet( ScStyleSheet* pNewStyle )
{
	if (pNewStyle)
	{
		SfxItemSet&		  rPatternSet = GetItemSet();
		const SfxItemSet& rStyleSet = pNewStyle->GetItemSet();

		for (sal_uInt16 i=ATTR_PATTERN_START; i<=ATTR_PATTERN_END; i++)
		{
			if (rStyleSet.GetItemState(i, sal_True) == SFX_ITEM_SET)
				rPatternSet.ClearItem(i);
		}
		rPatternSet.SetParent(&pNewStyle->GetItemSet());
		pStyle = pNewStyle;
		DELETEZ( pName );
	}
	else
	{
		DBG_ERROR( "ScPatternAttr::SetStyleSheet( NULL ) :-|" );
		GetItemSet().SetParent(NULL);
		pStyle = NULL;
	}
}

void ScPatternAttr::UpdateStyleSheet()
{
	if (pName)
	{
		pStyle = (ScStyleSheet*)pDoc->GetStyleSheetPool()->Find(*pName, SFX_STYLE_FAMILY_PARA);

		//	wenn Style nicht gefunden, Standard nehmen,
		//	damit keine leere Anzeige im Toolbox-Controller
		//!	es wird vorausgesetzt, dass "Standard" immer der erste Eintrag ist!
		if (!pStyle)
		{
			SfxStyleSheetIteratorPtr pIter = pDoc->GetStyleSheetPool()->CreateIterator( SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL );
			pStyle = dynamic_cast< ScStyleSheet* >(pIter->First());
		}

		if (pStyle)
		{
			GetItemSet().SetParent(&pStyle->GetItemSet());
			DELETEZ( pName );
		}
	}
	else
		pStyle = NULL;
}

void ScPatternAttr::StyleToName()
{
	// Style wurde geloescht, Namen merken:

	if ( pStyle )
	{
		if ( pName )
			*pName = pStyle->GetName();
		else
			pName = new String( pStyle->GetName() );

		pStyle = NULL;
		GetItemSet().SetParent( NULL );
	}
}

sal_Bool ScPatternAttr::IsSymbolFont() const
{
	const SfxPoolItem* pItem;
	if( GetItemSet().GetItemState( ATTR_FONT, sal_True, &pItem ) == SFX_ITEM_SET )
		return sal_Bool( ((const SvxFontItem*) pItem)->GetCharSet()
			== RTL_TEXTENCODING_SYMBOL );
	else
		return sal_False;
}

//UNUSED2008-05  FontToSubsFontConverter ScPatternAttr::GetSubsFontConverter( sal_uLong nFlags ) const
//UNUSED2008-05  {
//UNUSED2008-05      const SfxPoolItem* pItem;
//UNUSED2008-05      if( GetItemSet().GetItemState( ATTR_FONT, sal_True, &pItem ) == SFX_ITEM_SET )
//UNUSED2008-05          return CreateFontToSubsFontConverter(
//UNUSED2008-05              ((const SvxFontItem*) pItem)->GetFamilyName(), nFlags );
//UNUSED2008-05      else
//UNUSED2008-05          return 0;
//UNUSED2008-05  }


sal_uLong ScPatternAttr::GetNumberFormat( SvNumberFormatter* pFormatter ) const
{
	sal_uLong nFormat =
		((SfxUInt32Item*)&GetItemSet().Get( ATTR_VALUE_FORMAT ))->GetValue();
	LanguageType eLang =
		((SvxLanguageItem*)&GetItemSet().Get( ATTR_LANGUAGE_FORMAT ))->GetLanguage();
	if ( nFormat < SV_COUNTRY_LANGUAGE_OFFSET && eLang == LANGUAGE_SYSTEM )
		;		// es bleibt wie es ist
	else if ( pFormatter )
		nFormat = pFormatter->GetFormatForLanguageIfBuiltIn( nFormat, eLang );
	return nFormat;
}

//	dasselbe, wenn bedingte Formatierung im Spiel ist:

sal_uLong ScPatternAttr::GetNumberFormat( SvNumberFormatter* pFormatter,
										const SfxItemSet* pCondSet ) const
{
	DBG_ASSERT(pFormatter,"GetNumberFormat ohne Formatter");

	const SfxPoolItem* pFormItem;
	if ( !pCondSet || pCondSet->GetItemState(ATTR_VALUE_FORMAT,sal_True,&pFormItem) != SFX_ITEM_SET )
		pFormItem = &GetItemSet().Get(ATTR_VALUE_FORMAT);

	const SfxPoolItem* pLangItem;
	if ( !pCondSet || pCondSet->GetItemState(ATTR_LANGUAGE_FORMAT,sal_True,&pLangItem) != SFX_ITEM_SET )
		pLangItem = &GetItemSet().Get(ATTR_LANGUAGE_FORMAT);

	return pFormatter->GetFormatForLanguageIfBuiltIn(
					((SfxUInt32Item*)pFormItem)->GetValue(),
					((SvxLanguageItem*)pLangItem)->GetLanguage() );
}

const SfxPoolItem& ScPatternAttr::GetItem( sal_uInt16 nWhich, const SfxItemSet& rItemSet, const SfxItemSet* pCondSet )
{
    const SfxPoolItem* pCondItem;
    if ( pCondSet && pCondSet->GetItemState( nWhich, sal_True, &pCondItem ) == SFX_ITEM_SET )
        return *pCondItem;
    return rItemSet.Get(nWhich);
}

const SfxPoolItem& ScPatternAttr::GetItem( sal_uInt16 nSubWhich, const SfxItemSet* pCondSet ) const
{
    return GetItem( nSubWhich, GetItemSet(), pCondSet );
}

//	GetRotateVal testet vorher ATTR_ORIENTATION

long ScPatternAttr::GetRotateVal( const SfxItemSet* pCondSet ) const
{
	long nAttrRotate = 0;
    if ( GetCellOrientation() == SVX_ORIENTATION_STANDARD )
    {
        sal_Bool bRepeat = ( static_cast<const SvxHorJustifyItem&>(GetItem(ATTR_HOR_JUSTIFY, pCondSet)).
                            GetValue() == SVX_HOR_JUSTIFY_REPEAT );
        // ignore orientation/rotation if "repeat" is active
        if ( !bRepeat )
            nAttrRotate = ((const SfxInt32Item&)GetItem( ATTR_ROTATE_VALUE, pCondSet )).GetValue();
    }
	return nAttrRotate;
}

sal_uInt8 ScPatternAttr::GetRotateDir( const SfxItemSet* pCondSet ) const
{
	sal_uInt8 nRet = SC_ROTDIR_NONE;

	long nAttrRotate = GetRotateVal( pCondSet );
	if ( nAttrRotate )
	{
		SvxRotateMode eRotMode = (SvxRotateMode)((const SvxRotateModeItem&)
									GetItem(ATTR_ROTATE_MODE, pCondSet)).GetValue();

		if ( eRotMode == SVX_ROTATE_MODE_STANDARD || nAttrRotate == 18000 )
			nRet = SC_ROTDIR_STANDARD;
		else if ( eRotMode == SVX_ROTATE_MODE_CENTER )
			nRet = SC_ROTDIR_CENTER;
		else if ( eRotMode == SVX_ROTATE_MODE_TOP || eRotMode == SVX_ROTATE_MODE_BOTTOM )
		{
			long nRot180 = nAttrRotate % 18000;		// 1/100 Grad
			if ( nRot180 == 9000 )
				nRet = SC_ROTDIR_CENTER;
			else if ( ( eRotMode == SVX_ROTATE_MODE_TOP && nRot180 < 9000 ) ||
					  ( eRotMode == SVX_ROTATE_MODE_BOTTOM && nRot180 > 9000 ) )
				nRet = SC_ROTDIR_LEFT;
			else
				nRet = SC_ROTDIR_RIGHT;
		}
	}

	return nRet;
}




