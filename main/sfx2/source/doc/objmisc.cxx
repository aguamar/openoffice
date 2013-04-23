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
#include "precompiled_sfx2.hxx"

#ifndef _INETMSG_HXX //autogen
#include <svl/inetmsg.hxx>
#endif
#include <tools/diagnose_ex.h>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svtools/svparser.hxx> // SvKeyValue
#include <vos/mutex.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <com/sun/star/script/FinishEngineEvent.hpp>
#include <com/sun/star/script/InterruptReason.hpp>
#include <com/sun/star/script/XEngineListener.hpp>
#include <com/sun/star/script/XDebugging.hpp>
#ifndef _COM_SUN_STAR_SCRIPT_XINVOKATION_HPP_
#include <com/sun/star/script/XInvocation.hpp>
#endif
#include <com/sun/star/script/ContextInformation.hpp>
#include <com/sun/star/script/FinishReason.hpp>
#include <com/sun/star/script/XEngine.hpp>
#include <com/sun/star/script/InterruptEngineEvent.hpp>
#include <com/sun/star/script/XLibraryAccess.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>


#include <com/sun/star/script/provider/XScript.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <unotools/securityoptions.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/configurationhelper.hxx>

#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/task/DocumentMacroConfirmationRequest.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/frame/XModel.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::script::provider;
using namespace ::com::sun::star::container;
#include <basic/sbuno.hxx>
#include <basic/sbstar.hxx>
#ifndef _SB_BASMGR_HXX
#include <basic/basmgr.hxx>
#endif
#ifndef _VCL_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#include <basic/sbx.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>

#include <unotools/pathoptions.hxx>
#include <unotools/ucbhelper.hxx>
#include <tools/inetmime.hxx>
#include <tools/urlobj.hxx>
#include <svl/inettype.hxx>
#include <svl/sharecontrolfile.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/svapp.hxx>
#include <framework/interaction.hxx>
#include <framework/documentundoguard.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/documentconstants.hxx>

#include <sfx2/signaturestate.hxx>
#include <sfx2/app.hxx>
#include "appdata.hxx"
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/objsh.hxx>
#include "objshimp.hxx"
#include <sfx2/event.hxx>
#include "fltfnc.hxx"
#include <sfx2/sfx.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/ctrlitem.hxx>
#include "arrdecl.hxx"
#include <sfx2/module.hxx>
#include <sfx2/docfac.hxx>
#include "helper.hxx"
#include "doc.hrc"
#include "workwin.hxx"
#include "helpid.hrc"
#include "../appl/app.hrc"
#include <sfx2/sfxdlg.hxx>
#include "appbaslib.hxx"
#include <openflag.hxx>                 // SFX_STREAM_READWRITE

using namespace ::com::sun::star;

// class SfxHeaderAttributes_Impl ----------------------------------------

class SfxHeaderAttributes_Impl : public SvKeyValueIterator
{
private:
	SfxObjectShell* pDoc;
	SvKeyValueIteratorRef xIter;
	sal_Bool bAlert;

public:
	SfxHeaderAttributes_Impl( SfxObjectShell* pSh ) :
		SvKeyValueIterator(), pDoc( pSh ),
		xIter( pSh->GetMedium()->GetHeaderAttributes_Impl() ),
		bAlert( sal_False ) {}

	virtual sal_Bool GetFirst( SvKeyValue& rKV ) { return xIter->GetFirst( rKV ); }
	virtual sal_Bool GetNext( SvKeyValue& rKV ) { return xIter->GetNext( rKV ); }
	virtual void Append( const SvKeyValue& rKV );

	void ClearForSourceView() { xIter = new SvKeyValueIterator; bAlert = sal_False; }
	void SetAttributes();
	void SetAttribute( const SvKeyValue& rKV );
};

//=========================================================================

sal_uInt16 __READONLY_DATA aTitleMap_Impl[3][2] =
{
								//	local				remote
	/*	SFX_TITLE_CAPTION	*/	{ 	SFX_TITLE_FILENAME, SFX_TITLE_TITLE },
	/*	SFX_TITLE_PICKLIST  */	{ 	32,					SFX_TITLE_FULLNAME },
	/*	SFX_TITLE_HISTORY	*/	{ 	32,					SFX_TITLE_FULLNAME }
};

//=========================================================================

void SfxObjectShell::AbortImport()
{
	pImp->bIsAbortingImport = sal_True;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsAbortingImport() const
{
	return pImp->bIsAbortingImport;
}

//-------------------------------------------------------------------------

uno::Reference<document::XDocumentProperties>
SfxObjectShell::getDocProperties()
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties());
    DBG_ASSERT(xDocProps.is(),
        "SfxObjectShell: model has no DocumentProperties");
    return xDocProps;
}

//-------------------------------------------------------------------------

void SfxObjectShell::DoFlushDocInfo()
{
}

//-------------------------------------------------------------------------

// Note: the only thing that calls this is the modification event handler
// that is installed at the XDocumentProperties
void SfxObjectShell::FlushDocInfo()
{
    if ( IsLoading() )
        return;

	SetModified(sal_True);
    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());
    DoFlushDocInfo(); // call template method
    ::rtl::OUString url(xDocProps->getAutoloadURL());
    sal_Int32 delay(xDocProps->getAutoloadSecs());
    SetAutoLoad( INetURLObject(url), delay * 1000,
                 (delay > 0) || url.getLength() );
/*
	// bitte beachten:
	// 1. Titel in DocInfo aber nicht am Doc (nach HTML-Import)
	// 	=> auch am Doc setzen
	// 2. Titel in DocInfo leer (Briefumschlagsdruck)
	//	=> nicht am Doc setzen, da sonst "unbenanntX" daraus wird
	String aDocInfoTitle = GetDocInfo().GetTitle();
	if ( aDocInfoTitle.Len() )
		SetTitle( aDocInfoTitle );
	else
	{
		pImp->aTitle.Erase();
		SetNamedVisibility_Impl();
    	if ( GetMedium() )
    	{
        	SfxShell::SetName( GetTitle(SFX_TITLE_APINAME) );
        	Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
    	}
    }*/
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetError( sal_uInt32 lErr, const ::rtl::OUString& aLogMessage )
{
	if(pImp->lErr==ERRCODE_NONE)
    {
		pImp->lErr=lErr;

        if( lErr != ERRCODE_NONE && aLogMessage.getLength() )
            AddLog( aLogMessage );
    }
}

//-------------------------------------------------------------------------

sal_uInt32 SfxObjectShell::GetError() const
{
	return ERRCODE_TOERROR(GetErrorCode());
}

//-------------------------------------------------------------------------

sal_uInt32 SfxObjectShell::GetErrorCode() const
{
	sal_uInt32 lError=pImp->lErr;
	if(!lError && GetMedium())
		lError=GetMedium()->GetErrorCode();
	return lError;
}

//-------------------------------------------------------------------------

void SfxObjectShell::ResetError()
{
    if( pImp->lErr != ERRCODE_NONE )
        AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Resetting Error." ) ) );

	pImp->lErr=0;
	SfxMedium * pMed = GetMedium();
	if( pMed )
		pMed->ResetError();
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsTemplate() const
{
	return pImp->bIsTemplate;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetTemplate(sal_Bool bIs)
{
	pImp->bIsTemplate=bIs;
	SfxFilterMatcher aMatcher( GetFactory().GetFactoryName() );
	SfxFilterMatcherIter aIter( &aMatcher, SFX_FILTER_TEMPLATEPATH );
	SfxMedium* pMed = GetMedium();
	if( pMed ) pMed->SetFilter( aIter.First() );
}

//-------------------------------------------------------------------------

void SfxObjectShell::EnableSetModified( sal_Bool bEnable )
{
#ifdef DBG_UTIL
    if ( bEnable == pImp->m_bEnableSetModified )
        DBG_WARNING( "SFX_PERSIST: EnableSetModified 2x mit dem gleichen Wert gerufen" );
#endif
	pImp->m_bEnableSetModified = bEnable;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsEnableSetModified() const
{
    return pImp->m_bEnableSetModified && !IsReadOnly();
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsModified()
{
    if ( pImp->m_bIsModified )
        return sal_True;

    if ( !pImp->m_xDocStorage.is() || IsReadOnly() )
    {
        // if the document still has no storage and is not set to be modified explicitly it is not modified
        // a readonly document is also not modified

        return sal_False;
    }

    uno::Sequence < ::rtl::OUString > aNames = GetEmbeddedObjectContainer().GetObjectNames();
    for ( sal_Int32 n=0; n<aNames.getLength(); n++ )
    {
        uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObjectContainer().GetEmbeddedObject( aNames[n] );
        OSL_ENSURE( xObj.is(), "An empty entry in the embedded objects list!\n" );
        if ( xObj.is() )
        {
            try
            {
                sal_Int32 nState = xObj->getCurrentState();
                if ( nState != embed::EmbedStates::LOADED )
                {
                    uno::Reference< util::XModifiable > xModifiable( xObj->getComponent(), uno::UNO_QUERY );
                    if ( xModifiable.is() && xModifiable->isModified() )
                        return sal_True;
                }
            }
            catch( uno::Exception& )
            {}
        }
    }

    return sal_False;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetModified( sal_Bool bModifiedP )
{
#ifdef DBG_UTIL
    if ( !bModifiedP && !IsEnableSetModified() )
        DBG_WARNING( "SFX_PERSIST: SetModified( sal_False ), obwohl IsEnableSetModified() == sal_False" );
#endif

	if( !IsEnableSetModified() )
		return;

	if( pImp->m_bIsModified != bModifiedP )
	{
		pImp->m_bIsModified = bModifiedP;
		ModifyChanged();
	}
}

//-------------------------------------------------------------------------

void SfxObjectShell::ModifyChanged()
{
	if ( pImp->bClosing )
		// SetModified aus dem dispose des Models!
		return;

	{DBG_CHKTHIS(SfxObjectShell, 0);}

    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if ( pViewFrame )
        pViewFrame->GetBindings().Invalidate( SID_SAVEDOCS );

    Invalidate( SID_SIGNATURE );
    Invalidate( SID_MACRO_SIGNATURE );
	Broadcast( SfxSimpleHint( SFX_HINT_TITLECHANGED ) );	// xmlsec05, signed state might change in title...

    SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_MODIFYCHANGED, GlobalEventConfig::GetEventName(STR_EVENT_MODIFYCHANGED), this ) );
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsReadOnlyUI() const

/* 	[Beschreibung]

	Liefert sal_True, wenn das Dokument fuer die UI wie r/o behandelt werden
	soll. Dieses ist unabhaengig vom tatsaechlichen r/o, welches per
	<IsReadOnly()> erfragbar ist.
*/

{
	return pImp->bReadOnlyUI;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsReadOnlyMedium() const

/* 	[Beschreibung]

	Liefert sal_True, wenn das Medium r/o ist bzw. r/o geoeffnet wurde.
*/

{
	if ( !pMedium )
		return sal_True;
	return pMedium->IsReadOnly();
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetReadOnlyUI( sal_Bool bReadOnly )

/* 	[Beschreibung]

	Schaltet das Dokument in einen r/o bzw. r/w Zustand ohne es neu
	zu laden und ohne die Open-Modi des Mediums zu aendern.
*/

{
	sal_Bool bWasRO = IsReadOnly();
	pImp->bReadOnlyUI = bReadOnly;
	if ( bWasRO != IsReadOnly() )
	{
		Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );
		//if ( pImp->pDocInfo )
		//	pImp->pDocInfo->SetReadOnly( IsReadOnly() );
	}
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetReadOnly()
{
    // Let the document be completely readonly, means that the 
    // medium open mode is adjusted accordingly, and the write lock
    // on the file is removed.

 	if ( pMedium && !IsReadOnlyMedium() )
    {
        sal_Bool bWasROUI = IsReadOnly();

        pMedium->UnlockFile( sal_False );
 
        // the storage-based mediums are already based on the temporary file
        // so UnlockFile has already closed the locking stream
        if ( !pMedium->HasStorage_Impl() && IsLoadingFinished() )
            pMedium->CloseInStream();

        pMedium->SetOpenMode( SFX_STREAM_READONLY, pMedium->IsDirect(), sal_True ); 
        pMedium->GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );

        if ( !bWasROUI )
            Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );
    }
}
//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsReadOnly() const
{
	return pImp->bReadOnlyUI || IsReadOnlyMedium();
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsInModalMode() const
{
    return pImp->bModalMode || pImp->bRunningMacro;
}

//<!--Added by PengYunQuan for Validity Cell Range Picker
sal_Bool SfxObjectShell::AcceptStateUpdate() const
{
	return !IsInModalMode();
}
//-->Added by PengYunQuan for Validity Cell Range Picker

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::HasModalViews() const
{
	SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
	while( pFrame )
	{
		if ( pFrame->IsInModalMode() )
			return sal_True;

		pFrame = SfxViewFrame::GetNext( *pFrame, this );
	}

	return sal_False;
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetMacroMode_Impl( sal_Bool bModal )
{
    if ( !pImp->bRunningMacro != !bModal )
	{
        pImp->bRunningMacro = bModal;
		Broadcast( SfxSimpleHint( SFX_HINT_MODECHANGED ) );
	}
}

//-------------------------------------------------------------------------

void SfxObjectShell::SetModalMode_Impl( sal_Bool bModal )
{
	// nur Broadcasten wenn modifiziert, sonst ggf. Endlosrekursion
	if ( !pImp->bModalMode != !bModal )
	{
		// zentral mitz"ahlen
		sal_uInt16 &rDocModalCount = SFX_APP()->Get_Impl()->nDocModalMode;
		if ( bModal )
			++rDocModalCount;
		else
			--rDocModalCount;

		// umschalten
		pImp->bModalMode = bModal;
		Broadcast( SfxSimpleHint( SFX_HINT_MODECHANGED ) );
	}
}

//--------------------------------------------------------------------
sal_Bool SfxObjectShell::SwitchToShared( sal_Bool bShared, sal_Bool bSave )
{
    sal_Bool bResult = sal_True;

    if ( bShared != IsDocShared() )
    {
        ::rtl::OUString aOrigURL = GetMedium()->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );

        if ( !aOrigURL.getLength() && bSave )
        {
            // this is a new document, let it be stored before switching to the shared mode;
            // the storing should be done without shared flag, since it is possible that the
            // target location does not allow to create sharing control file;
            // the shared flag will be set later after creation of sharing control file
            SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst( this );

            if ( pViewFrame )
            {
                // TODO/LATER: currently the application guards against the reentrance problem
				const SfxPoolItem* pItem = pViewFrame->GetBindings().ExecuteSynchron( HasName() ? SID_SAVEDOC : SID_SAVEASDOC );
                SfxBoolItem* pResult = PTR_CAST( SfxBoolItem, pItem );
                bResult = ( pResult && pResult->GetValue() );
                if ( bResult )
                    aOrigURL = GetMedium()->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            }
        }

        sal_Bool bOldValue = HasSharedXMLFlagSet();
        SetSharedXMLFlag( bShared );

        sal_Bool bRemoveEntryOnError = sal_False;
        if ( bResult && bShared )
        {
            try
            {
                ::svt::ShareControlFile aControlFile( aOrigURL );
                aControlFile.InsertOwnEntry();
                bRemoveEntryOnError = sal_True;
            }
            catch( uno::Exception& )
            {
                bResult = sal_False;
            }
        }

        if ( bResult && bSave )
        {
            SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst( this );

            if ( pViewFrame )
            {
                // TODO/LATER: currently the application guards against the reentrance problem
                SetModified( sal_True ); // the modified flag has to be set to let the document be stored with the shared flag
				const SfxPoolItem* pItem = pViewFrame->GetBindings().ExecuteSynchron( HasName() ? SID_SAVEDOC : SID_SAVEASDOC );
                SfxBoolItem* pResult = PTR_CAST( SfxBoolItem, pItem );
                bResult = ( pResult && pResult->GetValue() );
            }
        }

        if ( bResult )
        {
            // TODO/LATER: Is it possible that the following calls fail?
            if ( bShared )
            {
                pImp->m_aSharedFileURL = aOrigURL;
                GetMedium()->SwitchDocumentToTempFile();
            }
            else
            {
                ::rtl::OUString aTempFileURL = pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
                GetMedium()->SwitchDocumentToFile( GetSharedFileURL() );
                pImp->m_aSharedFileURL = ::rtl::OUString();

                // now remove the temporary file the document was based on
                ::utl::UCBContentHelper::Kill( aTempFileURL );

                try
                {
                    // aOrigURL can not be used since it contains an old value
                    ::svt::ShareControlFile aControlFile( GetMedium()->GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
                    aControlFile.RemoveFile();
                }
                catch( uno::Exception& )
                {
                }
            }
        }
        else
        {
            // the saving has failed!
            if ( bRemoveEntryOnError )
            {
                try
                {
                    ::svt::ShareControlFile aControlFile( aOrigURL );
                    aControlFile.RemoveEntry();
                }
                catch( uno::Exception& )
                {}
            }

            SetSharedXMLFlag( bOldValue );
        }
    }
    else
        bResult = sal_False; // the second switch to the same mode

    if ( bResult )
        SetTitle( String() );

    return bResult;
}

//--------------------------------------------------------------------

void SfxObjectShell::DisconnectFromShared()
{
    if ( IsDocShared() )
    {
        if ( pMedium && pMedium->GetStorage().is() )
        {
            // set medium to noname
            pMedium->SetName( String(), sal_True );
            pMedium->Init_Impl();

            // drop resource
            SetNoName();
            InvalidateName();

            // untitled document must be based on temporary storage
            // the medium should not dispose the storage in this case
            if ( pMedium->GetStorage() == GetStorage() )
                ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium );

            pMedium->Close();
            FreeSharedFile();

            SfxMedium* pTmpMedium = pMedium;
            ForgetMedium();
            if( !DoSaveCompleted( pTmpMedium ) )
                SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
            else
            {
                // the medium should not dispose the storage, DoSaveCompleted() has let it to do so
                pMedium->CanDisposeStorage_Impl( sal_False );
            }

            pMedium->GetItemSet()->ClearItem( SID_DOC_READONLY );
            pMedium->SetOpenMode( SFX_STREAM_READWRITE, sal_True, sal_True );

            SetTitle( String() );
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::FreeSharedFile()
{
    if ( pMedium )
        FreeSharedFile( pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
}

//--------------------------------------------------------------------
void SfxObjectShell::FreeSharedFile( const ::rtl::OUString& aTempFileURL )
{
    SetSharedXMLFlag( sal_False );

    if ( IsDocShared() && aTempFileURL.getLength()
      && !::utl::UCBContentHelper::EqualURLs( aTempFileURL, GetSharedFileURL() ) )
    {
        if ( pImp->m_bAllowShareControlFileClean )
        {
            try
            {
                ::svt::ShareControlFile aControlFile( GetSharedFileURL() );
                aControlFile.RemoveEntry();
            }
            catch( uno::Exception& )
            {
            }
        }

        // the cleaning is forbidden only once
        pImp->m_bAllowShareControlFileClean = sal_True;

        // now remove the temporary file the document is based currently on
        ::utl::UCBContentHelper::Kill( aTempFileURL );

        pImp->m_aSharedFileURL = ::rtl::OUString();
    }
}

//--------------------------------------------------------------------
void SfxObjectShell::DoNotCleanShareControlFile()
{
    pImp->m_bAllowShareControlFileClean = sal_False;
}

//--------------------------------------------------------------------
void SfxObjectShell::SetSharedXMLFlag( sal_Bool bFlag ) const
{
    pImp->m_bSharedXMLFlag = bFlag;
}

//--------------------------------------------------------------------
sal_Bool SfxObjectShell::HasSharedXMLFlagSet() const
{
    return pImp->m_bSharedXMLFlag;
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShell::IsDocShared() const
{
    return ( pImp->m_aSharedFileURL.getLength() > 0 );
}

//--------------------------------------------------------------------

::rtl::OUString SfxObjectShell::GetSharedFileURL() const
{
    return pImp->m_aSharedFileURL;
}

//--------------------------------------------------------------------

Size SfxObjectShell::GetFirstPageSize()
{
    return GetVisArea(ASPECT_THUMBNAIL).GetSize();
}


//--------------------------------------------------------------------

IndexBitSet& SfxObjectShell::GetNoSet_Impl()
{
	return pImp->aBitSet;
}

//--------------------------------------------------------------------
// changes the title of the document

void SfxObjectShell::SetTitle
(
	const String& rTitle		// der neue Titel des Dokuments
)

/*	[Beschreibung]

	Mit dieser Methode kann der Titel des Dokuments gesetzt werden.
	Dieser entspricht initial dem kompletten Dateinamen. Ein Setzen
	des Titels wirkt jedoch nicht zu"uck auf den Dateinamen; er wird
	jedoch in den Caption-Bars der MDI-Fenster angezeigt.
*/

{
	DBG_CHKTHIS(SfxObjectShell, 0);

	// nix zu tun?
	if ( ( ( HasName() && pImp->aTitle == rTitle )
		|| ( !HasName() && GetTitle() == rTitle ) )
	  && !IsDocShared() )
		return;

	SfxApplication *pSfxApp = SFX_APP();
#if 0
	// wird 'unbenannt#' als Titel gesetzt
	String aNoName(SfxResId(STR_NONAME));
	if ( rTitle.Match(aNoName) <= aNoName.Len() )
	{
		// er ist es selbst => ignorieren
		pSfxApp->ReleaseIndex(pImp->nVisualDocumentNumber);
		pImp->bIsNamedVisible=0;
	}
#endif

	// ggf. die unbenannt-Nummer freigeben
	if ( pImp->bIsNamedVisible && USHRT_MAX != pImp->nVisualDocumentNumber )
	{
		pSfxApp->ReleaseIndex(pImp->nVisualDocumentNumber);
		pImp->bIsNamedVisible = 0;
	}

	// Title setzen
	pImp->aTitle = rTitle;
//  Wieso denn in der DocInfo?
//	GetDocInfo().SetTitle( rTitle );
//	FlushDocInfo();

	// Benachrichtigungen
    if ( GetMedium() )
    {
        SfxShell::SetName( GetTitle(SFX_TITLE_APINAME) );
        Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
    }
}

//--------------------------------------------------------------------

#if OSL_DEBUG_LEVEL > 1
String X(const String &rRet)
{
	if ( !rRet.Len() )
		return DEFINE_CONST_UNICODE( "-empty-" );
	return rRet;
}
#else
#define X(ret) ret
#endif

//--------------------------------------------------------------------
//--------------------------------------------------------------------
String SfxObjectShell::GetTitle
(
	sal_uInt16	nMaxLength 		/*	0 (default)
								der Titel selbst, so wie er ist

								1 (==SFX_TITLE_FILENAME)
								liefert den logischen Dateinamen ohne Pfad
								(unter WNT je nach Systemeinstellung ohne
								Extension)

								2 (==SFX_TITLE_FULLNAME)
								liefert den mit komplettem logischen Dateinamen
								mit Pfad (remote => ::com::sun::star::util::URL)

								3 (==SFX_TITLE_APINAME)
								liefert den logischen Dateinamen ohne Pfad
								und Extension

								4 (==SFX_TITLE_DETECT)
								liefert den kompletten Titel, falls noch
								nicht gesetzt wird aber aus DocInfo oder
								dem Namen des Medium erzeugt

								5 (==SFX_TITLE_CAPTION)
								liefert den Titel so, wie MB ihn heute in
								der CaptionBar anzeigen m"ochte

								6 (==SFX_TITLE_PICKLIST)
								liefert den Titel so, wie MB ihn heute in
								der PickList anzeigen m"ochte

								7 (==SFX_TITLE_HISTORY)
								liefert den Titel so, wie MB ihn heute in
								der History anzeigen m"ochte

								10 bis USHRT_MAX
								liefert maximal 'nMaxLength' Zeichen vom logischen
								Dateinamen inkl. Pfad (remote => ::com::sun::star::util::URL)
								*/
) const

/*	[Beschreibung]

	Liefert den Titel bzw. logischen Dateinamen des Dokuments, je nach
	'nMaxLength'.

	Falls der Dateiname mit Pfad verwendet wird, wird die Namensk"urzung durch
	Ersetzung eines oder mehrerer Directory-Namen durch "..." durchgef"uhrt,
	URLs werden z.Zt. immer komplett geliefert.
*/

{
//    if ( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
//        return String();
    SfxMedium *pMed = GetMedium();
    if ( IsLoading() )
        return String();

/*    if ( !nMaxLength && pImp->pDocInfo )
    {
        String aTitle = pImp->pDocInfo->GetTitle();
        if ( aTitle.Len() )
            return aTitle;
    } */

	// Titel erzeugen?
	if ( SFX_TITLE_DETECT == nMaxLength && !pImp->aTitle.Len() )
	{
		static sal_Bool bRecur = sal_False;
		if ( bRecur )
			return DEFINE_CONST_UNICODE( "-not available-" );
		bRecur = sal_True;

		String aTitle;
		SfxObjectShell *pThis = (SfxObjectShell*) this;

		if ( pMed )
		{
			SFX_ITEMSET_ARG( pMed->GetItemSet(), pNameItem, SfxStringItem, SID_DOCINFO_TITLE, sal_False );
			if ( pNameItem )
				aTitle = pNameItem->GetValue();
		}

		if ( !aTitle.Len() )
			aTitle = GetTitle( SFX_TITLE_FILENAME );

		if ( IsTemplate() )
			pThis->SetTitle( aTitle );
		bRecur = sal_False;
		return X(aTitle);
	}
	else if (SFX_TITLE_APINAME == nMaxLength )
		return X(GetAPIName());

	// Sonderfall Vorlagen:
	if( IsTemplate() && pImp->aTitle.Len() &&
		 ( nMaxLength == SFX_TITLE_CAPTION || nMaxLength == SFX_TITLE_PICKLIST ) )
		return X(pImp->aTitle);

	// Picklist/Caption wird gemappt
	if ( pMed && ( nMaxLength == SFX_TITLE_CAPTION || nMaxLength == SFX_TITLE_PICKLIST ) )
	{
		// Wenn ein spezieller Titel beim "Offnen mitgegeben wurde;
		// wichtig bei URLs, die INET_PROT_FILE verwenden, denn bei denen
		// wird der gesetzte Titel nicht beachtet.
		// (s.u., Auswertung von aTitleMap_Impl)
		SFX_ITEMSET_ARG( pMed->GetItemSet(), pNameItem, SfxStringItem, SID_DOCINFO_TITLE, sal_False );
		if ( pNameItem )
			return X( pNameItem->GetValue() );
	}

	// noch unbenannt?
	DBG_ASSERT( !HasName() || pMed, "HasName() aber kein Medium?!?" );
	if ( !HasName() || !pMed )
	{
		// schon Titel gesezt?
		if ( pImp->aTitle.Len() )
			return X(pImp->aTitle);

		// mu\s es durchnumeriert werden?
		String aNoName( SfxResId( STR_NONAME ) );
		if ( pImp->bIsNamedVisible )
			// Nummer hintenanh"angen
			aNoName += String::CreateFromInt32( pImp->nVisualDocumentNumber );

		// Dokument hei\st vorerst 'unbenannt#'
		return X(aNoName);
	}

	const INetURLObject aURL( IsDocShared() ? GetSharedFileURL() : ::rtl::OUString( GetMedium()->GetName() ) );
	if ( nMaxLength > SFX_TITLE_CAPTION && nMaxLength <= SFX_TITLE_HISTORY )
	{
		sal_uInt16 nRemote;
		if( !pMed || aURL.GetProtocol() == INET_PROT_FILE )
			nRemote = 0;
		else
			nRemote = 1;
		nMaxLength = aTitleMap_Impl[nMaxLength-SFX_TITLE_CAPTION][nRemote];
	}

	// lokale Datei?
	if ( aURL.GetProtocol() == INET_PROT_FILE )
	{
        String aName( aURL.HasMark() ? INetURLObject( aURL.GetURLNoMark() ).PathToFileName() : aURL.PathToFileName() );
		if ( nMaxLength == SFX_TITLE_FULLNAME )
			return X( aName );
		else if ( nMaxLength == SFX_TITLE_FILENAME )
            return X( aURL.getName( INetURLObject::LAST_SEGMENT,
				true, INetURLObject::DECODE_WITH_CHARSET ) );
		else if ( !pImp->aTitle.Len() )
            pImp->aTitle = aURL.getBase( INetURLObject::LAST_SEGMENT,
										 true, INetURLObject::DECODE_WITH_CHARSET );
	}
	else
	{
		// ::com::sun::star::util::URL-Versionen
		if ( nMaxLength >= SFX_TITLE_MAXLEN )
		{
            String aComplete( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
			if( aComplete.Len() > nMaxLength )
			{
				String aRet( DEFINE_CONST_UNICODE( "..." ) );
				aRet += aComplete.Copy( aComplete.Len() - nMaxLength + 3, nMaxLength - 3 );
				return X( aRet );
			}
			else
                return X( aComplete );
		}
		else if ( nMaxLength == SFX_TITLE_FILENAME )
		{
            String aName( aURL.GetBase() );
			aName = INetURLObject::decode( aName, INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET );
			if( !aName.Len() )
				aName = aURL.GetURLNoPass();
			return X(aName);
		}
		else if ( nMaxLength == SFX_TITLE_FULLNAME )
			return X(aURL.GetMainURL( INetURLObject::DECODE_TO_IURI ));

		// ggf. Titel aus Dateiname generieren
		if ( !pImp->aTitle.Len() )
			pImp->aTitle = aURL.GetBase();

		// workaround for the case when the name can not be retrieved from URL by INetURLObject
		if ( !pImp->aTitle.Len() )
			pImp->aTitle = aURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );
	}

	// ganzer Titel
	return X(pImp->aTitle);
}

//--------------------------------------------------------------------

void SfxObjectShell::InvalidateName()

/*	[Beschreibung]

	Ermittelt den Titel des Dokuments neu aus 'unbenannt', DocInfo-Titel
	bzw. Dateinamen. Wird nach Laden aus Template oder SaveAs ben"otigt.
*/

{
	// Title neu erzeugen
	pImp->aTitle.Erase();
//	pImp->nVisualDocumentNumber = USHRT_MAX;
	//GetTitle( SFX_TITLE_DETECT );
	SetName( GetTitle( SFX_TITLE_APINAME ) );

	// Benachrichtigungen
	Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
}

//--------------------------------------------------------------------

void SfxObjectShell::SetNamedVisibility_Impl()
{
	if ( !pImp->bIsNamedVisible )
	{
		// Nummer verpassen
		pImp->bIsNamedVisible = sal_True;
		// ggf. neue Nummer verpassen
		if ( !HasName() && USHRT_MAX == pImp->nVisualDocumentNumber && !pImp->aTitle.Len() )
		{
			pImp->nVisualDocumentNumber = SFX_APP()->GetFreeIndex();
			Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
		}
	}

	SetName( GetTitle(SFX_TITLE_APINAME) );
}

void SfxObjectShell::SetNoName()
{
    bHasName = 0;
    bIsTmp = sal_True;
    GetModel()->attachResource( ::rtl::OUString(), GetModel()->getArgs() );
}

//--------------------------------------------------------------------

void SfxObjectShell::MemoryError()
{
}

//--------------------------------------------------------------------

SfxProgress* SfxObjectShell::GetProgress() const
{
	return pImp->pProgress;
}

//--------------------------------------------------------------------

void SfxObjectShell::SetProgress_Impl
(
	SfxProgress *pProgress	/*	zu startender <SfxProgress> oder 0, falls
								der Progress zur"uckgesetzt werden soll */
)

/*	[Beschreibung]

	Interne Methode zum setzen oder zur"ucksetzen des Progress-Modes
	f"ur diese SfxObjectShell.
*/

{
	DBG_ASSERT( ( !pImp->pProgress && pProgress ) ||
				( pImp->pProgress && !pProgress ),
				"Progress activation/deacitivation mismatch" );
	pImp->pProgress = pProgress;
}

//--------------------------------------------------------------------

void SfxObjectShell::PostActivateEvent_Impl( SfxViewFrame* pFrame )
{
	SfxApplication* pSfxApp = SFX_APP();
    if ( !pSfxApp->IsDowning() && !IsLoading() && pFrame && !pFrame->GetFrame().IsClosing_Impl() )
	{
        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pHiddenItem, SfxBoolItem, SID_HIDDEN, sal_False );
        if ( !pHiddenItem || !pHiddenItem->GetValue() )
        {
            sal_uInt16 nId = pImp->nEventId;
            pImp->nEventId = 0;
            if ( nId == SFX_EVENT_OPENDOC )
                pSfxApp->NotifyEvent(SfxViewEventHint( nId, GlobalEventConfig::GetEventName(STR_EVENT_OPENDOC), this, pFrame->GetFrame().GetController() ), sal_False);
			else if (nId == SFX_EVENT_CREATEDOC )
                pSfxApp->NotifyEvent(SfxViewEventHint( nId, GlobalEventConfig::GetEventName(STR_EVENT_CREATEDOC), this, pFrame->GetFrame().GetController() ), sal_False);
        }
	}
}

//--------------------------------------------------------------------

void SfxObjectShell::SetActivateEvent_Impl(sal_uInt16 nId )
{
	if ( GetFactory().GetFlags() & SFXOBJECTSHELL_HASOPENDOC )
		pImp->nEventId = nId;
}

//--------------------------------------------------------------------

void SfxObjectShell::RegisterTransfer( SfxMedium& rMedium )
/*  [Beschreibung ]
	Alle Medien, die aufgesetzt werden, um Teile eines Dokumentes zu
	laden, muessen an der zugehoerigen SfxObjectShell angemeldet
	werden. So kann dokumentweise abgebrochen werden.  */
{
	rMedium.SetReferer( GetMedium()->GetName() );
}

//-------------------------------------------------------------------------

void SfxObjectShell::PrepareReload( )
/*  [Beschreibung ]
	Wird vor dem Reload gerufen und gibt die Moeglichkeit,
	etwaige Caches zu leeren. */
{
}

//-------------------------------------------------------------------------

void SfxObjectShell::LockAutoLoad( sal_Bool bLock )

/* 	Verhindert ein evtl. eintreffendes AutoLoad. Wird auch vor AutoLoad
	eines umgebenden FrameSet beruecksichtigt.
*/

{
	if ( bLock )
		++pImp->nAutoLoadLocks;
	else
		--pImp->nAutoLoadLocks;
}

//-------------------------------------------------------------------------

// kann nach frame.cxx gemoved werden, wenn 358+36x-Stand gemerged sind

sal_Bool SfxFrame::IsAutoLoadLocked_Impl() const
{
	// sein einges Doc gelockt?
	const SfxObjectShell* pObjSh = GetCurrentDocument();
	if ( !pObjSh || !pObjSh->IsAutoLoadLocked() )
		return sal_False;

	// seine Childs gelockt?
	for ( sal_uInt16 n = GetChildFrameCount(); n--; )
		if ( !GetChildFrame(n)->IsAutoLoadLocked_Impl() )
			return sal_False;

	// sonst ist AutoLoad erlaubt
	return sal_True;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsAutoLoadLocked() const

/* 	Liefert, ob ein eintreffendes AutoLoad ausgefuehrt werden darf. Wird auch
	vor AutoLoad eines umgebenden FrameSet beruecksichtigt.
*/

{
	return !IsReadOnly() || pImp->nAutoLoadLocks > 0;
}

//-------------------------------------------------------------------------
void SfxObjectShell::BreakMacroSign_Impl( sal_Bool bBreakMacroSign )
{
	pImp->m_bMacroSignBroken = bBreakMacroSign;
}

//-------------------------------------------------------------------------
void SfxObjectShell::CheckSecurityOnLoading_Impl()
{
    uno::Reference< task::XInteractionHandler > xInteraction;
    if ( GetMedium() )
        xInteraction = GetMedium()->GetInteractionHandler();

	// check if there is a broken signature...
    CheckForBrokenDocSignatures_Impl( xInteraction );

    CheckEncryption_Impl( xInteraction );

    // check macro security
    pImp->aMacroMode.checkMacrosOnLoading( xInteraction );
}

//-------------------------------------------------------------------------
void SfxObjectShell::CheckEncryption_Impl( const uno::Reference< task::XInteractionHandler >& xHandler )
{
    ::rtl::OUString aVersion;
    sal_Bool bIsEncrypted = sal_False;
    sal_Bool bHasNonEncrypted = sal_False;

    try
    {
        uno::Reference < beans::XPropertySet > xPropSet( GetStorage(), uno::UNO_QUERY_THROW );
        xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Version" ) ) ) >>= aVersion;
        xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasEncryptedEntries" ) ) ) >>= bIsEncrypted;
        xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasNonEncryptedEntries" ) ) ) >>= bHasNonEncrypted;
    }
    catch( uno::Exception& )
    {
    }

    if ( aVersion.compareTo( ODFVER_012_TEXT ) >= 0 )
    {
        // this is ODF1.2 or later
        if ( bIsEncrypted && bHasNonEncrypted )
        {
            if ( !pImp->m_bIncomplEncrWarnShown )
            {
                // this is an encrypted document with nonencrypted streams inside, show the warning
                ::com::sun::star::task::ErrorCodeRequest aErrorCode;
                aErrorCode.ErrCode = ERRCODE_SFX_INCOMPLETE_ENCRYPTION;

                SfxMedium::CallApproveHandler( xHandler, uno::makeAny( aErrorCode ), sal_False );
                pImp->m_bIncomplEncrWarnShown = sal_True;
            }

            // broken signatures imply no macro execution at all
            pImp->aMacroMode.disallowMacroExecution();
        }
    }
}

//-------------------------------------------------------------------------
void SfxObjectShell::CheckForBrokenDocSignatures_Impl( const uno::Reference< task::XInteractionHandler >& xHandler )
{
    sal_Int16 nSignatureState = GetDocumentSignatureState();
    bool bSignatureBroken = ( nSignatureState == SIGNATURESTATE_SIGNATURES_BROKEN );
    if ( !bSignatureBroken )
        return;

    pImp->showBrokenSignatureWarning( xHandler );

    // broken signatures imply no macro execution at all
    pImp->aMacroMode.disallowMacroExecution();
}

//-------------------------------------------------------------------------
void SfxObjectShell::SetAutoLoad(
	const INetURLObject& rUrl, sal_uInt32 nTime, sal_Bool bReload )
{
	if ( pImp->pReloadTimer )
		DELETEZ(pImp->pReloadTimer);
	if ( bReload )
	{
		pImp->pReloadTimer = new AutoReloadTimer_Impl(
								rUrl.GetMainURL( INetURLObject::DECODE_TO_IURI ),
								nTime, bReload, this );
		pImp->pReloadTimer->Start();
	}
}

sal_Bool SfxObjectShell::IsLoadingFinished() const
{
	return ( pImp->nLoadedFlags == SFX_LOADED_ALL );
}

void impl_addToModelCollection(const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& xModel);
void SfxObjectShell::InitOwnModel_Impl()
{
	if ( !pImp->bModelInitialized )
	{
		SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
		if ( pSalvageItem )
		{
            pImp->aTempName = pMedium->GetPhysicalName();
			pMedium->GetItemSet()->ClearItem( SID_DOC_SALVAGE );
			pMedium->GetItemSet()->ClearItem( SID_FILE_NAME );
			pMedium->GetItemSet()->Put( SfxStringItem( SID_FILE_NAME, pMedium->GetOrigURL() ) );
		}
		else
		{
			pMedium->GetItemSet()->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
			pMedium->GetItemSet()->ClearItem( SID_DOCUMENT );
		}

		pMedium->GetItemSet()->ClearItem( SID_REFERER );
		uno::Reference< frame::XModel >  xModel ( GetModel(), uno::UNO_QUERY );
		if ( xModel.is() )
		{
			::rtl::OUString aURL = GetMedium()->GetOrigURL();
			SfxItemSet *pSet = GetMedium()->GetItemSet();
            if ( !GetMedium()->IsReadOnly() )
                pSet->ClearItem( SID_INPUTSTREAM );
			uno::Sequence< beans::PropertyValue > aArgs;
			TransformItems( SID_OPENDOC, *pSet, aArgs );
			xModel->attachResource( aURL, aArgs );
            impl_addToModelCollection(xModel);
		}

		pImp->bModelInitialized = sal_True;
	}
}

void SfxObjectShell::FinishedLoading( sal_uInt16 nFlags )
{
	sal_Bool bSetModifiedTRUE = sal_False;
	SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False );
    if( ( nFlags & SFX_LOADED_MAINDOCUMENT ) && !(pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT )
	    && !(pImp->nFlagsInProgress & SFX_LOADED_MAINDOCUMENT ))
	{
		pImp->nFlagsInProgress |= SFX_LOADED_MAINDOCUMENT;
		((SfxHeaderAttributes_Impl*)GetHeaderAttributes())->SetAttributes();
		pImp->bImportDone = sal_True;
		if( !IsAbortingImport() )
			PositionView_Impl();

        if ( ( GetModifyPasswordHash() || GetModifyPasswordInfo().getLength() ) && !IsModifyPasswordEntered() )
            SetReadOnly();

		// Salvage
		if ( pSalvageItem )
			bSetModifiedTRUE = sal_True;

		if ( !IsEnableSetModified() )
			EnableSetModified( sal_True );

		if( !bSetModifiedTRUE && IsEnableSetModified() )
			SetModified( sal_False );

        CheckSecurityOnLoading_Impl();

		bHasName = sal_True; // the document is loaded, so the name should already available
		GetTitle( SFX_TITLE_DETECT );
		InitOwnModel_Impl();
		pImp->nFlagsInProgress &= ~SFX_LOADED_MAINDOCUMENT;
	}

    if( ( nFlags & SFX_LOADED_IMAGES ) && !(pImp->nLoadedFlags & SFX_LOADED_IMAGES )
	    && !(pImp->nFlagsInProgress & SFX_LOADED_IMAGES ))
	{
		pImp->nFlagsInProgress |= SFX_LOADED_IMAGES;
        uno::Reference<document::XDocumentProperties> xDocProps(
            getDocProperties());
        ::rtl::OUString url(xDocProps->getAutoloadURL());
        sal_Int32 delay(xDocProps->getAutoloadSecs());
        SetAutoLoad( INetURLObject(url), delay * 1000,
                     (delay > 0) || url.getLength() );
		if( !bSetModifiedTRUE && IsEnableSetModified() )
			SetModified( sal_False );
		Invalidate( SID_SAVEASDOC );
		pImp->nFlagsInProgress &= ~SFX_LOADED_IMAGES;
	}

	pImp->nLoadedFlags |= nFlags;

	if ( !pImp->nFlagsInProgress )
	{
		// in case of reentrance calls the first called FinishedLoading() call on the stack
		// should do the notification, in result the notification is done when all the FinishedLoading() calls are finished

		if ( bSetModifiedTRUE )
			SetModified( sal_True );
		else
			SetModified( sal_False );

		if ( (pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) && (pImp->nLoadedFlags & SFX_LOADED_IMAGES ) )
		{
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pTemplateItem, SfxBoolItem, SID_TEMPLATE, sal_False);
            sal_Bool bTemplate = pTemplateItem && pTemplateItem->GetValue();

			// closing the streams on loading should be under control of SFX!
			DBG_ASSERT( pMedium->IsOpen(), "Don't close the medium when loading documents!" );

            if ( bTemplate )
            {
                TemplateDisconnectionAfterLoad();
            }
            else
            {
                // if a readonly medium has storage then it's stream is already based on temporary file
                if( !(pMedium->GetOpenMode() & STREAM_WRITE) && !pMedium->HasStorage_Impl() )
                    // don't lock file opened read only
                    pMedium->CloseInStream();
            }
		}

        SetInitialized_Impl( false );

		// Title is not available until loading has finished
		Broadcast( SfxSimpleHint( SFX_HINT_TITLECHANGED ) );
		if ( pImp->nEventId )
			PostActivateEvent_Impl(SfxViewFrame::GetFirst(this));
	}
}

//-------------------------------------------------------------------------
extern void SetTemplate_Impl( const String&, const String&, SfxObjectShell* );

void SfxObjectShell::TemplateDisconnectionAfterLoad()
{
    // document is created from a template
    //TODO/LATER: should the templates always be XML docs!

    SfxMedium* pTmpMedium = pMedium;
    if ( pTmpMedium )
    {
        String aName( pTmpMedium->GetName() );
        SFX_ITEMSET_ARG( pTmpMedium->GetItemSet(), pTemplNamItem, SfxStringItem, SID_TEMPLATE_NAME, sal_False);
        String aTemplateName;
        if ( pTemplNamItem )
            aTemplateName = pTemplNamItem->GetValue();
        else
        {
            // !TODO/LATER: what's this?!
            // Interaktiv ( DClick, Contextmenu ) kommt kein Langname mit
            aTemplateName = getDocProperties()->getTitle();
            if ( !aTemplateName.Len() )
            {
                INetURLObject aURL( aName );
                aURL.CutExtension();
                aTemplateName = aURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
            }
        }

        // set medium to noname
        pTmpMedium->SetName( String(), sal_True );
        pTmpMedium->Init_Impl();

        // drop resource
        SetNoName();
        InvalidateName();

        if( IsPackageStorageFormat_Impl( *pTmpMedium ) )
        {
            // untitled document must be based on temporary storage
            // the medium should not dispose the storage in this case
            uno::Reference < embed::XStorage > xTmpStor = ::comphelper::OStorageHelper::GetTemporaryStorage();
            GetStorage()->copyToStorage( xTmpStor );

            // the medium should disconnect from the original location
            // the storage should not be disposed since the document is still
            // based on it, but in DoSaveCompleted it will be disposed
            pTmpMedium->CanDisposeStorage_Impl( sal_False );
            pTmpMedium->Close();

            // setting the new storage the medium will be based on
            pTmpMedium->SetStorage_Impl( xTmpStor );

            ForgetMedium();
            if( !DoSaveCompleted( pTmpMedium ) )
                SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
            else
            {
                SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False );
                sal_Bool bSalvage = pSalvageItem ? sal_True : sal_False;

                if ( !bSalvage )
                {
                    // some further initializations for templates
                    SetTemplate_Impl( aName, aTemplateName, this );
                }

                // the medium should not dispose the storage, DoSaveCompleted() has let it to do so
                pTmpMedium->CanDisposeStorage_Impl( sal_False );
            }
        }
        else
        {
            // some further initializations for templates
            SetTemplate_Impl( aName, aTemplateName, this );
            pTmpMedium->CreateTempFile( sal_True );
        }

        // templates are never readonly
        pTmpMedium->GetItemSet()->ClearItem( SID_DOC_READONLY );
        pTmpMedium->SetOpenMode( SFX_STREAM_READWRITE, sal_True, sal_True );

        // notifications about possible changes in readonly state and document info
        Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );

        // created untitled document can't be modified
        SetModified( sal_False );
    }
}

//-------------------------------------------------------------------------

void SfxObjectShell::PositionView_Impl()
{
	MarkData_Impl *pMark = Get_Impl()->pMarkData;
	if( pMark )
	{
		SfxViewShell* pSh = pMark->pFrame->GetViewShell();
		if( pMark->aUserData.Len() )
			pSh->ReadUserData( pMark->aUserData, sal_True );
		else if( pMark->aMark.Len() )
			pSh->JumpToMark( pMark->aMark );
		DELETEZ( Get_Impl()->pMarkData );
	}
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsLoading() const
/*  [Beschreibung ]
	Has FinishedLoading been called? */
{
	return !( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT );
}

//-------------------------------------------------------------------------

void SfxObjectShell::CancelTransfers()
/*  [Beschreibung ]
	Hier koennen Transfers gecanceled werden, die nicht mit
	RegisterTransfer registiert wurden */
{
	if( ( pImp->nLoadedFlags & SFX_LOADED_ALL ) != SFX_LOADED_ALL )
	{
		AbortImport();
		if( IsLoading() )
			FinishedLoading( SFX_LOADED_ALL );

/*
		SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
		while( pFrame )
		{
			pFrame->CancelTransfers();
			pFrame = SfxViewFrame::GetNext( *pFrame, this );
		}*/
	}
}

//-------------------------------------------------------------------------

AutoReloadTimer_Impl::AutoReloadTimer_Impl(
	const String& rURL, sal_uInt32 nTime, sal_Bool bReloadP, SfxObjectShell* pSh )
	: aUrl( rURL ), bReload( bReloadP ), pObjSh( pSh )
{
	SetTimeout( nTime );
}

//-------------------------------------------------------------------------

void AutoReloadTimer_Impl::Timeout()
{
	SfxViewFrame *pFrame = SfxViewFrame::GetFirst( pObjSh );

	if ( pFrame )
	{
		// momentan nicht m"oglich/sinnvoll?
        if ( !pObjSh->CanReload_Impl() || pObjSh->IsAutoLoadLocked() || Application::IsUICaptured() )
		{
			// erneuten Versuch erlauben
			Start();
			return;
		}

		SfxAllItemSet aSet( SFX_APP()->GetPool() );
		aSet.Put( SfxBoolItem( SID_AUTOLOAD, sal_True ) );
		if ( aUrl.Len() )
			aSet.Put(  SfxStringItem( SID_FILE_NAME, aUrl ) );
		SfxRequest aReq( SID_RELOAD, 0, aSet );
		pObjSh->Get_Impl()->pReloadTimer = 0;
		delete this;
		pFrame->ExecReload_Impl( aReq );
		return;
	}

	pObjSh->Get_Impl()->pReloadTimer = 0;
	delete this;
}

SfxModule* SfxObjectShell::GetModule() const
{
	return GetFactory().GetModule();
}

ErrCode SfxObjectShell::CallBasic( const String& rMacro,
	const String& rBasic, SbxArray* pArgs,
	SbxValue* pRet )
{
    SfxApplication* pApp = SFX_APP();
    if( pApp->GetName() != rBasic )
    {
        if ( !AdjustMacroMode( String() ) )
            return ERRCODE_IO_ACCESSDENIED;
    }

    BasicManager *pMgr = GetBasicManager();
    if( pApp->GetName() == rBasic )
        pMgr = pApp->GetBasicManager();
    ErrCode nRet = SfxApplication::CallBasic( rMacro, pMgr, pArgs, pRet );
    return nRet;
}

namespace
{
    static bool lcl_isScriptAccessAllowed_nothrow( const Reference< XInterface >& _rxScriptContext )
    {
        try
        {
            Reference< XEmbeddedScripts > xScripts( _rxScriptContext, UNO_QUERY );
            if ( !xScripts.is() )
            {
                Reference< XScriptInvocationContext > xContext( _rxScriptContext, UNO_QUERY_THROW );
                xScripts.set( xContext->getScriptContainer(), UNO_SET_THROW );
            }

            return xScripts->getAllowMacroExecution();
        }
        catch( const Exception& )
        {
        	DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }
}

ErrCode SfxObjectShell::CallXScript( const Reference< XInterface >& _rxScriptContext, const ::rtl::OUString& _rScriptURL,
    const Sequence< Any >& aParams, Any& aRet, Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam, bool bRaiseError )
{
    OSL_TRACE( "in CallXScript" );
	ErrCode nErr = ERRCODE_NONE;

    bool bIsDocumentScript = ( _rScriptURL.indexOfAsciiL( RTL_CONSTASCII_STRINGPARAM( "location=document" ) ) >= 0 );
        // TODO: we should parse the URL, and check whether there is a parameter with this name.
        // Otherwise, we might find too much.
    if ( bIsDocumentScript && !lcl_isScriptAccessAllowed_nothrow( _rxScriptContext ) )
        return ERRCODE_IO_ACCESSDENIED;

	bool bCaughtException = false;
    Any aException;
    try
    {
        // obtain/create a script provider
        Reference< provider::XScriptProvider > xScriptProvider;
        Reference< provider::XScriptProviderSupplier > xSPS( _rxScriptContext, UNO_QUERY );
        if ( xSPS.is() )
            xScriptProvider.set( xSPS->getScriptProvider() );

        if ( !xScriptProvider.is() )
        {
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            Reference< provider::XScriptProviderFactory > xScriptProviderFactory(
                aContext.getSingleton( "com.sun.star.script.provider.theMasterScriptProviderFactory" ), UNO_QUERY_THROW );
            xScriptProvider.set( xScriptProviderFactory->createScriptProvider( makeAny( _rxScriptContext ) ), UNO_SET_THROW );
        }

        // ry to protect the invocation context's undo manager (if present), just in case the script tampers with it
        ::framework::DocumentUndoGuard aUndoGuard( _rxScriptContext.get() );

        // obtain the script, and execute it
        Reference< provider::XScript > xScript( xScriptProvider->getScript( _rScriptURL ), UNO_QUERY_THROW );
        aRet = xScript->invoke( aParams, aOutParamIndex, aOutParam );
    }
    catch ( const uno::Exception& )
    {
        aException = ::cppu::getCaughtException();
		bCaughtException = sal_True;
        nErr = ERRCODE_BASIC_INTERNAL_ERROR;
    }

	if ( bCaughtException && bRaiseError )
	{
        ::std::auto_ptr< VclAbstractDialog > pScriptErrDlg;
		SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
        if ( pFact )
            pScriptErrDlg.reset( pFact->CreateScriptErrorDialog( NULL, aException ) );
        OSL_ENSURE( pScriptErrDlg.get(), "SfxObjectShell::CallXScript: no script error dialog!" );

        if ( pScriptErrDlg.get() )
			pScriptErrDlg->Execute();
	}

    OSL_TRACE( "leaving CallXScript" );
    return nErr;
}

// perhaps rename to CallScript once we get rid of the existing CallScript
// and Call, CallBasic, CallStarBasic methods
ErrCode SfxObjectShell::CallXScript( const String& rScriptURL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >&
            aParams,
        ::com::sun::star::uno::Any& aRet,
        ::com::sun::star::uno::Sequence< sal_Int16 >& aOutParamIndex,
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam
        , bool bRaiseError )
{
    return CallXScript( GetModel(), rScriptURL, aParams, aRet, aOutParamIndex, aOutParam, bRaiseError );
}

//-------------------------------------------------------------------------
SfxFrame* SfxObjectShell::GetSmartSelf( SfxFrame* pSelf, SfxMedium& /*rMedium*/ )
{
	return pSelf;
}

SfxObjectShellFlags SfxObjectShell::GetFlags() const
{
	if( pImp->eFlags == SFXOBJECTSHELL_UNDEFINED )
		pImp->eFlags = GetFactory().GetFlags();
	return pImp->eFlags;
}

void SfxObjectShell::SetFlags( SfxObjectShellFlags eFlags )
{
	pImp->eFlags = eFlags;
}

void SfxHeaderAttributes_Impl::SetAttributes()
{
	bAlert = sal_True;
	SvKeyValue aPair;
	for( sal_Bool bCont = xIter->GetFirst( aPair ); bCont;
		 bCont = xIter->GetNext( aPair ) )
		SetAttribute( aPair );
}

void SfxHeaderAttributes_Impl::SetAttribute( const SvKeyValue& rKV )
{
	String aValue = rKV.GetValue();
	if( rKV.GetKey().CompareIgnoreCaseToAscii( "refresh" ) == COMPARE_EQUAL && rKV.GetValue().Len() )
	{
		sal_uInt32 nTime = aValue.GetToken(  0, ';' ).ToInt32() ;
		String aURL = aValue.GetToken( 1, ';' );
		aURL.EraseTrailingChars().EraseLeadingChars();
        uno::Reference<document::XDocumentProperties> xDocProps(
            pDoc->getDocProperties());
		if( aURL.Copy(0, 4).CompareIgnoreCaseToAscii( "url=" ) == COMPARE_EQUAL )
		{
			INetURLObject aObj;
			INetURLObject( pDoc->GetMedium()->GetName() ).GetNewAbsURL( aURL.Copy( 4 ), &aObj );
            xDocProps->setAutoloadURL(
                aObj.GetMainURL( INetURLObject::NO_DECODE ) );
		}
        try
        {
            xDocProps->setAutoloadSecs( nTime );
        }
        catch (lang::IllegalArgumentException &)
        {
            // ignore
        }
	}
	else if( rKV.GetKey().CompareIgnoreCaseToAscii( "expires" ) == COMPARE_EQUAL )
	{
		DateTime aDateTime;
		if( INetRFC822Message::ParseDateField( rKV.GetValue(), aDateTime ) )
		{
			aDateTime.ConvertToLocalTime();
			pDoc->GetMedium()->SetExpired_Impl( aDateTime );
		}
		else
		{
//			DBG_ERROR( "Schlechtes ::com::sun::star::util::DateTime fuer Expired" );
			pDoc->GetMedium()->SetExpired_Impl( Date( 1, 1, 1970 ) );
		}
	}
	else if( rKV.GetKey().CompareIgnoreCaseToAscii( "content-type" ) == COMPARE_EQUAL )
	{
		::rtl::OString sContent = ::rtl::OUStringToOString( aValue, RTL_TEXTENCODING_ASCII_US );
		ByteString sType, sSubType;
		INetContentTypeParameterList aParameters;

		if( INetContentTypes::parse( sContent, sType, sSubType, &aParameters ) )
		{
			const INetContentTypeParameter * pCharset = aParameters.find("charset");
			if (pCharset != 0)
				pDoc->GetMedium()->SetCharset( pCharset->m_sValue );
		}
	}
}

void SfxHeaderAttributes_Impl::Append( const SvKeyValue& rKV )
{
	xIter->Append( rKV );
	if( bAlert ) SetAttribute( rKV );
}

SvKeyValueIterator* SfxObjectShell::GetHeaderAttributes()
{
	if( !pImp->xHeaderAttributes.Is() )
	{
		DBG_ASSERT( pMedium, "Kein Medium" );
		pImp->xHeaderAttributes = new SfxHeaderAttributes_Impl( this );
	}
	return ( SvKeyValueIterator*) &pImp->xHeaderAttributes;
}

void SfxObjectShell::ClearHeaderAttributesForSourceViewHack()
{
	((SfxHeaderAttributes_Impl*)GetHeaderAttributes())
		->ClearForSourceView();
}


void SfxObjectShell::SetHeaderAttributesForSourceViewHack()
{
	((SfxHeaderAttributes_Impl*)GetHeaderAttributes())
		->SetAttributes();
}

sal_Bool SfxObjectShell::IsPreview() const
{
	if ( !pMedium )
		return sal_False;

	sal_Bool bPreview = sal_False;
	SFX_ITEMSET_ARG( pMedium->GetItemSet(), pFlags, SfxStringItem, SID_OPTIONS, sal_False);
	if ( pFlags )
	{
		// Werte auf einzelne Items verteilen
		String aFileFlags = pFlags->GetValue();
		aFileFlags.ToUpperAscii();
		if ( STRING_NOTFOUND != aFileFlags.Search( 'B' ) )
			bPreview = sal_True;
	}

	if ( !bPreview )
	{
		SFX_ITEMSET_ARG( pMedium->GetItemSet(), pItem, SfxBoolItem, SID_PREVIEW, sal_False);
		if ( pItem )
			bPreview = pItem->GetValue();
	}

	return bPreview;
}

sal_Bool SfxObjectShell::IsSecure()
{
	// Wenn globale Warnung an ist, nach Secure-Referer-Liste gehen
	String aReferer = GetMedium()->GetName();
	if ( !aReferer.Len() )
	{
		// bei neuen Dokumenten das Template als Referer nehmen
		::rtl::OUString aTempl( getDocProperties()->getTemplateURL() );
		if ( aTempl.getLength() )
            aReferer = INetURLObject( aTempl ).GetMainURL( INetURLObject::NO_DECODE );
	}

	INetURLObject aURL( "macro:" );
    if ( !aReferer.Len() )
        // empty new or embedded document
        return sal_True;

		SvtSecurityOptions aOpt;

	if( aOpt.GetBasicMode() == eALWAYS_EXECUTE )
    	return sal_True;

	if( aOpt.GetBasicMode() == eNEVER_EXECUTE )
    	return sal_False;

	if ( aOpt.IsSecureURL( aURL.GetMainURL( INetURLObject::NO_DECODE ), aReferer ) )
    //if ( SvtSecurityOptions().IsSecureURL( aURL.GetMainURL( INetURLObject::NO_DECODE ), aReferer ) )
	{
        if ( GetMedium()->GetContent().is() )
        {
            Any aAny( ::utl::UCBContentHelper::GetProperty( aURL.GetMainURL( INetURLObject::NO_DECODE ), String( RTL_CONSTASCII_USTRINGPARAM("IsProtected")) ) );
            sal_Bool bIsProtected = sal_False;
            if ( ( aAny >>= bIsProtected ) && bIsProtected )
                return sal_False;
            else
                return sal_True;
		}
		else
			return sal_True;
	}
	else
		return sal_False;
}

void SfxObjectShell::SetWaitCursor( sal_Bool bSet ) const
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this ); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, this ) )
    {
        if ( bSet )
            pFrame->GetFrame().GetWindow().EnterWait();
        else
            pFrame->GetFrame().GetWindow().LeaveWait();
    }
}

String SfxObjectShell::GetAPIName() const
{
	INetURLObject aURL( IsDocShared() ? GetSharedFileURL() : ::rtl::OUString( GetMedium()->GetName() ) );
    String aName( aURL.GetBase() );
    if( !aName.Len() )
        aName = aURL.GetURLNoPass();
    if ( !aName.Len() )
        aName = GetTitle( SFX_TITLE_DETECT );
    return aName;
}

void SfxObjectShell::Invalidate( sal_uInt16 nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this ); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, this ) )
        Invalidate_Impl( pFrame->GetBindings(), nId );
}

bool SfxObjectShell::AdjustMacroMode( const String& /*rScriptType*/, bool bSuppressUI )
{
    uno::Reference< task::XInteractionHandler > xInteraction;
    if ( pMedium && !bSuppressUI )
        xInteraction = pMedium->GetInteractionHandler();

    CheckForBrokenDocSignatures_Impl( xInteraction );

    CheckEncryption_Impl( xInteraction );

    return pImp->aMacroMode.adjustMacroMode( xInteraction );
}

Window* SfxObjectShell::GetDialogParent( SfxMedium* pLoadingMedium )
{
    Window* pWindow = 0;
    SfxItemSet* pSet = pLoadingMedium ? pLoadingMedium->GetItemSet() : GetMedium()->GetItemSet();
    SFX_ITEMSET_ARG( pSet, pUnoItem, SfxUnoFrameItem, SID_FILLFRAME, sal_False );
    if ( pUnoItem )
    {
        uno::Reference < frame::XFrame > xFrame( pUnoItem->GetFrame() );
        pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
    }

    if ( !pWindow )
    {
        SfxFrame* pFrame = 0;
        SFX_ITEMSET_ARG( pSet, pFrameItem, SfxFrameItem, SID_DOCFRAME, sal_False );
        if( pFrameItem && pFrameItem->GetFrame() )
            // get target frame from ItemSet
            pFrame = pFrameItem->GetFrame();
        else
        {
            // try the current frame
            SfxViewFrame* pView = SfxViewFrame::Current();
            if ( !pView || pView->GetObjectShell() != this )
                // get any visible frame
                pView = SfxViewFrame::GetFirst(this);
            if ( pView )
                pFrame = &pView->GetFrame();
        }

        if ( pFrame )
            // get topmost window
            pWindow = VCLUnoHelper::GetWindow( pFrame->GetFrameInterface()->getContainerWindow() );
    }

    if ( pWindow )
    {
        // this frame may be invisible, show it if it is allowed
        SFX_ITEMSET_ARG( pSet, pHiddenItem, SfxBoolItem, SID_HIDDEN, sal_False );
        if ( !pHiddenItem || !pHiddenItem->GetValue() )
        {
            pWindow->Show();
            pWindow->ToTop();
        }
    }

    return pWindow;
}

String SfxObjectShell::UpdateTitle( SfxMedium* pMed, sal_uInt16 nDocViewNumber )
{
    // Titel des Fensters
    String aTitle;
    if ( pMed )
	{
		INetURLObject aTmp( pMed->GetName() );
        aTitle = aTmp.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
	}
    else
    {
		pMed = GetMedium();
        aTitle = GetTitle(SFX_TITLE_CAPTION);
        String aName(aTitle);
        if ( nDocViewNumber )
        {
            aName += ':';
            aName += String::CreateFromInt32( nDocViewNumber );
        }
    }

	if ( pMed )
	{
		SFX_ITEMSET_ARG( pMed->GetItemSet(), pRepairedDocItem, SfxBoolItem, SID_REPAIRPACKAGE, sal_False );
		if ( pRepairedDocItem && pRepairedDocItem->GetValue() )
        	aTitle += String( SfxResId(STR_REPAIREDDOCUMENT) );
	}

	if ( IsReadOnlyUI() || (pMed && pMed->IsReadOnly()) )
        aTitle += String( SfxResId(STR_READONLY) );
    else if ( IsDocShared() )
        aTitle += String( SfxResId(STR_SHARED) );

    return aTitle;
}

void SfxObjectShell::SetCreateMode_Impl( SfxObjectCreateMode nMode )
{
	eCreateMode = nMode;
}

sal_Bool SfxObjectShell::IsInPlaceActive()
{
	if ( eCreateMode != SFX_CREATE_MODE_EMBEDDED )
		return sal_False;

    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
    return pFrame && pFrame->GetFrame().IsInPlace();
}

sal_Bool SfxObjectShell::IsUIActive()
{
	if ( eCreateMode != SFX_CREATE_MODE_EMBEDDED )
		return sal_False;

    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
    return pFrame && pFrame->GetFrame().IsInPlace() && pFrame->GetFrame().GetWorkWindow_Impl()->IsVisible_Impl();
}

void SfxObjectShell::UIActivate( sal_Bool )
{
}

void SfxObjectShell::InPlaceActivate( sal_Bool )
{
}

sal_Bool SfxObjectShell::UseInteractionToHandleError(
                    const uno::Reference< task::XInteractionHandler >& xHandler,
                    sal_uInt32 nError )
{
    sal_Bool bResult = sal_False;

    if ( xHandler.is() )
    {
        try
        {
            uno::Any aInteraction;
            uno::Sequence< uno::Reference< task::XInteractionContinuation > > lContinuations(2);
            ::comphelper::OInteractionAbort* pAbort = new ::comphelper::OInteractionAbort();
            ::comphelper::OInteractionApprove* pApprove = new ::comphelper::OInteractionApprove();
            lContinuations[0] = uno::Reference< task::XInteractionContinuation >(
                                 static_cast< task::XInteractionContinuation* >( pAbort ), uno::UNO_QUERY );
            lContinuations[1] = uno::Reference< task::XInteractionContinuation >(
                                 static_cast< task::XInteractionContinuation* >( pApprove ), uno::UNO_QUERY );

            task::ErrorCodeRequest aErrorCode;
            aErrorCode.ErrCode = nError;
            aInteraction <<= aErrorCode;
            xHandler->handle(::framework::InteractionRequest::CreateRequest (aInteraction,lContinuations));
            bResult = pAbort->wasSelected();
        }
        catch( uno::Exception& )
        {}
    }

    return bResult;
}

sal_Bool SfxObjectShell_Impl::NeedsOfficeUpdateDialog()
{
    // if the configuration is not available for any reason, the default behavior is to show the message
    sal_Bool bResult = sal_True;

    try
    {
    	uno::Reference< lang::XMultiServiceFactory > xServiceManager( ::comphelper::getProcessServiceFactory(), uno::UNO_SET_THROW );
		uno::Reference< uno::XInterface > xCommonConfig(
                        ::comphelper::ConfigurationHelper::openConfig(
							xServiceManager,
							::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Common" ) ),
							::comphelper::ConfigurationHelper::E_STANDARD ),
                        uno::UNO_SET_THROW );

        ::comphelper::ConfigurationHelper::readRelativeKey(
                        xCommonConfig,
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Load/" ) ),
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ShowOfficeUpdateDialog" ) ) ) >>= bResult;
    }
    catch( uno::Exception& )
    {
    }

	return bResult;
}

sal_Int16 SfxObjectShell_Impl::getCurrentMacroExecMode() const
{
    sal_Int16 nImposedExecMode( MacroExecMode::NEVER_EXECUTE );

    const SfxMedium* pMedium( rDocShell.GetMedium() );
    OSL_PRECOND( pMedium, "SfxObjectShell_Impl::getCurrentMacroExecMode: no medium!" );
    if ( pMedium )
    {
        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pMacroModeItem, SfxUInt16Item, SID_MACROEXECMODE, sal_False);
        if ( pMacroModeItem )
            nImposedExecMode = pMacroModeItem->GetValue();
    }
    return nImposedExecMode;
}

sal_Bool SfxObjectShell_Impl::setCurrentMacroExecMode( sal_uInt16 nMacroMode )
{
    const SfxMedium* pMedium( rDocShell.GetMedium() );
    OSL_PRECOND( pMedium, "SfxObjectShell_Impl::getCurrentMacroExecMode: no medium!" );
    if ( pMedium )
    {
		pMedium->GetItemSet()->Put( SfxUInt16Item( SID_MACROEXECMODE, nMacroMode ) );
        return sal_True;
    }

    return sal_False;
}

::rtl::OUString SfxObjectShell_Impl::getDocumentLocation() const
{
    ::rtl::OUString sLocation;

    const SfxMedium* pMedium( rDocShell.GetMedium() );
    OSL_PRECOND( pMedium, "SfxObjectShell_Impl::getDocumentLocation: no medium!" );
    if ( pMedium )
    {
        sLocation = pMedium->GetName();
        if ( !sLocation.getLength() )
        {
            // for documents made from a template: get the name of the template
            sLocation = rDocShell.getDocProperties()->getTemplateURL();
        }
    }
    return sLocation;
}

uno::Reference< embed::XStorage > SfxObjectShell_Impl::getZipStorageToSign()
{
    Reference < embed::XStorage > xStore;

    SfxMedium* pMedium( rDocShell.GetMedium() );
    OSL_PRECOND( pMedium, "SfxObjectShell_Impl::getLastCommitDocumentStorage: no medium!" );
    if ( pMedium )
        xStore = pMedium->GetZipStorageToSign_Impl();

    return xStore;
}

sal_Bool SfxObjectShell_Impl::documentStorageHasMacros() const
{
    return ::sfx2::DocumentMacroMode::storageHasMacros( m_xDocStorage );
}

Reference< XEmbeddedScripts > SfxObjectShell_Impl::getEmbeddedDocumentScripts() const
{
    return Reference< XEmbeddedScripts >( rDocShell.GetModel(), UNO_QUERY );
}

sal_Int16 SfxObjectShell_Impl::getScriptingSignatureState()
{
    sal_Int16 nSignatureState( rDocShell.GetScriptingSignatureState() );

    if ( nSignatureState != SIGNATURESTATE_NOSIGNATURES && m_bMacroSignBroken )
    {
        // if there is a macro signature it must be handled as broken
        nSignatureState = SIGNATURESTATE_SIGNATURES_BROKEN;
    }

    return nSignatureState;
}

sal_Bool SfxObjectShell_Impl::hasTrustedScriptingSignature( sal_Bool bAllowUIToAddAuthor )
{
    sal_Bool bResult = sal_False;

    try
    {
        ::rtl::OUString aVersion;
        try
        {
            uno::Reference < beans::XPropertySet > xPropSet( rDocShell.GetStorage(), uno::UNO_QUERY_THROW );
            xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Version" ) ) ) >>= aVersion;
        }
        catch( uno::Exception& )
        {
        }
        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[0] <<= aVersion;

        uno::Reference< security::XDocumentDigitalSignatures > xSigner( comphelper::getProcessServiceFactory()->createInstanceWithArguments( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ), aArgs ), uno::UNO_QUERY_THROW );

        if ( nScriptingSignatureState == SIGNATURESTATE_UNKNOWN
          || nScriptingSignatureState == SIGNATURESTATE_SIGNATURES_OK
          || nScriptingSignatureState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED )
        {
            uno::Sequence< security::DocumentSignatureInformation > aInfo = rDocShell.ImplAnalyzeSignature( sal_True, xSigner );

            if ( aInfo.getLength() )
            {
                if ( nScriptingSignatureState == SIGNATURESTATE_UNKNOWN )
                    nScriptingSignatureState = rDocShell.ImplCheckSignaturesInformation( aInfo );

                if ( nScriptingSignatureState == SIGNATURESTATE_SIGNATURES_OK
                  || nScriptingSignatureState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED )
                {
                    for ( sal_Int32 nInd = 0; !bResult && nInd < aInfo.getLength(); nInd++ )
                    {
                        bResult = xSigner->isAuthorTrusted( aInfo[nInd].Signer );
                    }

                    if ( !bResult && bAllowUIToAddAuthor )
                    {
                        uno::Reference< task::XInteractionHandler > xInteraction;
                        if ( rDocShell.GetMedium() )
                            xInteraction = rDocShell.GetMedium()->GetInteractionHandler();

                        if ( xInteraction.is() )
                        {
                            task::DocumentMacroConfirmationRequest aRequest;
                            aRequest.DocumentURL = getDocumentLocation();
                            aRequest.DocumentStorage = rDocShell.GetMedium()->GetZipStorageToSign_Impl();
                            aRequest.DocumentSignatureInformation = aInfo;
                            aRequest.DocumentVersion = aVersion;
                            aRequest.Classification = task::InteractionClassification_QUERY;
                            bResult = SfxMedium::CallApproveHandler( xInteraction, uno::makeAny( aRequest ), sal_True );
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception& )
    {}

    return bResult;
}

void SfxObjectShell_Impl::showBrokenSignatureWarning( const uno::Reference< task::XInteractionHandler >& _rxInteraction ) const
{
    if  ( !bSignatureErrorIsShown )
    {
        SfxObjectShell::UseInteractionToHandleError( _rxInteraction, ERRCODE_SFX_BROKENSIGNATURE );
	    const_cast< SfxObjectShell_Impl* >( this )->bSignatureErrorIsShown = sal_True;
    }
}

void SfxObjectShell::AddLog( const ::rtl::OUString& aMessage )
{
    if ( !pImp->m_xLogRing.is() )
    {
        try
        {
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            if ( aContext.is() )
                pImp->m_xLogRing.set( aContext.getSingleton( "com.sun.star.logging.DocumentIOLogRing" ), UNO_QUERY_THROW );
        }
        catch( uno::Exception& )
        {}
    }

    if ( pImp->m_xLogRing.is() )
        pImp->m_xLogRing->logString( aMessage );
}

namespace {

void WriteStringInStream( const uno::Reference< io::XOutputStream >& xOutStream, const ::rtl::OUString& aString )
{
    if ( xOutStream.is() )
    {
        ::rtl::OString aStrLog = ::rtl::OUStringToOString( aString, RTL_TEXTENCODING_UTF8 );
        uno::Sequence< sal_Int8 > aLogData( (const sal_Int8*)aStrLog.getStr(), aStrLog.getLength() );
        xOutStream->writeBytes( aLogData );

        aLogData.realloc( 1 );
        aLogData[0] = '\n';
        xOutStream->writeBytes( aLogData );
    }
}

}

void SfxObjectShell::StoreLog()
{
    if ( !pImp->m_xLogRing.is() )
    {
        try
        {
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            if ( aContext.is() )
                pImp->m_xLogRing.set( aContext.getSingleton( "com.sun.star.logging.DocumentIOLogRing" ), UNO_QUERY_THROW );
        }
        catch( uno::Exception& )
        {}
    }

    if ( pImp->m_xLogRing.is() )
    {
        ::rtl::OUString aFileURL = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}" ) );
//#ifdef WNT
//        ::rtl::OUString aFileURL = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}" ) );
//#else
//        ::rtl::OUString aFileURL = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "${$OOO_BASE_DIR/program/bootstraprc:UserInstallation}" ) );
//#endif
        ::rtl::Bootstrap::expandMacros( aFileURL );

//#ifdef WNT
//        ::rtl::OUString aBuildID = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "${$OOO_BASE_DIR/program/setup.ini:buildid}" ) );
//#else
//        ::rtl::OUString aBuildID = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "${$OOO_BASE_DIR/program/setuprc:buildid}" ) );
//#endif
        ::rtl::OUString aBuildID = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("setup") ":buildid}" ) );
        ::rtl::Bootstrap::expandMacros( aBuildID );

        if ( aFileURL.getLength() )
        {
            aFileURL += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/temp/document_io_logring.txt" ) );
            try
            {
                uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory(), uno::UNO_SET_THROW );
                uno::Reference< ucb::XSimpleFileAccess > xSimpleFileAccess( xFactory->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.ucb.SimpleFileAccess" ) ), uno::UNO_QUERY_THROW );
                uno::Reference< io::XStream > xStream( xSimpleFileAccess->openFileReadWrite( aFileURL ), uno::UNO_SET_THROW );
                uno::Reference< io::XOutputStream > xOutStream( xStream->getOutputStream(), uno::UNO_SET_THROW );
                uno::Reference< io::XTruncate > xTruncate( xOutStream, uno::UNO_QUERY_THROW );
                xTruncate->truncate();

                if ( aBuildID.getLength() )
                    WriteStringInStream( xOutStream, aBuildID );

                uno::Sequence< ::rtl::OUString > aLogSeq = pImp->m_xLogRing->getCollectedLog();
                for ( sal_Int32 nInd = 0; nInd < aLogSeq.getLength(); nInd++ )
                    WriteStringInStream( xOutStream, aLogSeq[nInd] );
            }
            catch( uno::Exception& )
            {}
        }
    }
}

