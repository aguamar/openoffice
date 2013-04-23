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




#ifndef _SD_TPACTION_HXX
#define _SD_TPACTION_HXX

#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <svtools/filedlg.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <svx/dlgctrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include "sdtreelb.hxx"
#include "anmdef.hxx"

#include <vector>

namespace sd {
    class View;
}
class SdDrawDocument;

/*************************************************************************
|*
|* Effekte-SingleTab-Dialog
|*
\************************************************************************/
class SdActionDlg : public SfxSingleTabDialog
{
private:
	const SfxItemSet&	rOutAttrs;

public:

			SdActionDlg( Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView );
			~SdActionDlg() {};
};

/*************************************************************************
|*
|* Interaktions-Tab-Page
|*
\************************************************************************/

class SdTPAction : public SfxTabPage
{
private:
	FixedText			    aFtAction;				// always visible 
	ListBox  			    aLbAction;
                                     
	FixedText   		    aFtTree;				// jump destination controls
	SdPageObjsTLB  		    aLbTree;
	SdPageObjsTLB  		    aLbTreeDocument;
    ListBox					aLbOLEAction;                        

	FixedLine               aFlSeparator;
	Edit					aEdtSound;
	Edit                    aEdtBookmark;
	Edit                    aEdtDocument;
	Edit                    aEdtProgram;
	Edit                    aEdtMacro;
	PushButton              aBtnSearch;
	PushButton              aBtnSeek;
                            
	const SfxItemSet&	    rOutAttrs;
	const ::sd::View*		mpView;
	SdDrawDocument*		    mpDoc;
	XColorList*		    pColTab;
                            
	sal_Bool				    bTreeUpdated;
	List*   			    pCurrentActions;
	String				    aLastFile;
    ::std::vector< long >   aVerbVector;

	//------------------------------------

	DECL_LINK( ClickSearchHdl, void * );
	DECL_LINK( ClickActionHdl, void * );
	DECL_LINK( SelectTreeHdl, void * );
	DECL_LINK( CheckFileHdl, void * );

	void					UpdateTree();
	virtual void 			OpenFileDialog();
	::com::sun::star::presentation::ClickAction 	GetActualClickAction();
	void 					SetActualClickAction( ::com::sun::star::presentation::ClickAction eCA );
	void 					SetActualAnimationEffect( ::com::sun::star::presentation::AnimationEffect eAE );
	void 					SetEditText( String const & rStr );
	String					GetEditText( sal_Bool bURL = sal_False );
	sal_uInt16          		GetClickActionSdResId( ::com::sun::star::presentation::ClickAction eCA );
	sal_uInt16          		GetAnimationEffectSdResId( ::com::sun::star::presentation::AnimationEffect eAE );

public:
			SdTPAction( Window* pParent, const SfxItemSet& rInAttrs );
			~SdTPAction();

	static  SfxTabPage* Create( Window*, const SfxItemSet& );

	virtual sal_Bool FillItemSet( SfxItemSet& );
	virtual void Reset( const SfxItemSet & );

	virtual void ActivatePage( const SfxItemSet& rSet );
	virtual int  DeactivatePage( SfxItemSet* pSet );

	void    Construct();

	void	SetView( const ::sd::View* pSdView );

	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;
};

#endif		// _SD_TPACTION_HXX

