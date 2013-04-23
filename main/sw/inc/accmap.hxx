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


#ifndef _ACCMAP_HXX
#define _ACCMAP_HXX

#include <cppuhelper/weakref.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <vos/ref.hxx>
#include <vos/mutex.hxx>
#include <svx/IAccessibleViewForwarder.hxx>
#include <svx/IAccessibleParent.hxx>
#include <tools/debug.hxx>
#include <tools/fract.hxx>

//IAccessibility2 Implementation 2009-----
#include <svx/AccessibleControlShape.hxx>
#include <svx/AccessibleShape.hxx>
#include "fesh.hxx"
#include <vector>
#include <set>
class SwAccessibleParagraph;
//-----IAccessibility2 Implementation 2009

class ViewShell;
class Rectangle;
class SwFrm;
class SwTxtFrm;
class SwPageFrm;
class SwAccessibleContext;
class SwAccessibleContextMap_Impl;
class SwAccessibleEventList_Impl;
class SwAccessibleEventMap_Impl;
class SwShapeList_Impl;
class SdrObject;
namespace accessibility {
    class AccessibleShape;
}
class SwAccessibleShapeMap_Impl;
struct SwAccessibleEvent_Impl;
class SwAccessibleSelectedParas_Impl;
class SwRect;
class MapMode;
class SwAccPreviewData;
struct PrevwPage;
class Window;

// real states for events
#define ACC_STATE_EDITABLE 0x01
#define ACC_STATE_OPAQUE 0x02

// pseudo states for events
// --> OD 2009-01-07 #i88069# - pseudo state for event TEXT_ATTRIBUTE_CHANGED
#define ACC_STATE_TEXT_ATTRIBUTE_CHANGED 0x0200
// <--
// --> OD 2005-12-12 #i27301# - pseudo state for event TEXT_SELECTION_CHANGED
#define ACC_STATE_TEXT_SELECTION_CHANGED 0x0100
// <--
#define ACC_STATE_CARET 0x80
#define ACC_STATE_RELATION_FROM 0x40
#define ACC_STATE_RELATION_TO 0x20

#define ACC_STATE_RELATION_MASK 0x60

#define ACC_STATE_MASK 0x1F

// --> OD 2005-12-12 #i27301# - introduce type definition of states
typedef sal_uInt16 tAccessibleStates;
// <--

class SwAccessibleMap : public ::accessibility::IAccessibleViewForwarder,
						public ::accessibility::IAccessibleParent
{
    mutable ::vos::OMutex maMutex;
	::vos::OMutex maEventMutex;
	SwAccessibleContextMap_Impl *mpFrmMap;
	SwAccessibleShapeMap_Impl *mpShapeMap;
	SwShapeList_Impl *mpShapes;
	SwAccessibleEventList_Impl *mpEvents;
	SwAccessibleEventMap_Impl *mpEventMap;
    // --> OD 2005-12-13 #i27301# - data structure to keep information about
    // accessible paragraph, which have a selection.
    SwAccessibleSelectedParas_Impl* mpSelectedParas;
    // <--
    ViewShell *mpVSh;
    /// for page preview: store preview data, VisArea, and mapping of
    /// preview-to-display coordinates
    SwAccPreviewData* mpPreview;

	::com::sun::star::uno::WeakReference < ::com::sun::star::accessibility::XAccessible > mxCursorContext;

	sal_Int32 mnPara;
	sal_Int32 mnFootnote;
	sal_Int32 mnEndnote;


	sal_Bool mbShapeSelected;

	void FireEvent( const SwAccessibleEvent_Impl& rEvent );

	void AppendEvent( const SwAccessibleEvent_Impl& rEvent );

	void InvalidateCursorPosition(
		const ::com::sun::star::uno::Reference<
			::com::sun::star::accessibility::XAccessible>& rAcc );
	//IAccessibility2 Implementation 2009-----
	void DoInvalidateShapeSelection(sal_Bool bInvalidateFocusMode = sal_False);
	
	//Replace by DoInvalidateShapeSelection
	//void DoInvalidateShapeFocus();
	void InvalidateShapeSelection();

	//mpSelectedFrmMap contains the old selected objects.
	SwAccessibleContextMap_Impl *mpSeletedFrmMap;
	//IvalidateShapeInParaSelection() method is reponsible for the updating the selected states of the objects.
	void InvalidateShapeInParaSelection();
	//-----IAccessibility2 Implementation 2009

    void _InvalidateRelationSet( const SwFrm* pFrm, sal_Bool bFrom );

	::com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessible>
            _GetDocumentView( sal_Bool bPagePreview );

    /** method to build up a new data structure of the accessible pararaphs,
        which have a selection

        OD 2005-12-13 #i27301#
        Important note: method has to used inside a mutual exclusive section

        @author OD
    */
    SwAccessibleSelectedParas_Impl* _BuildSelectedParas();

public:

	SwAccessibleMap( ViewShell *pSh );
	~SwAccessibleMap();

	::com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessible> GetDocumentView();

    // OD 15.01.2003 #103492# - complete re-factoring of method due to new
    // page/print preview functionality.
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> GetDocumentPreview(
                            const std::vector<PrevwPage*>& _rPrevwPages,
                            const Fraction&  _rScale,
                            const SwPageFrm* _pSelectedPageFrm,
                            const Size&      _rPrevwWinSize );

	::vos::ORef < SwAccessibleContext > GetContextImpl(
									 			const SwFrm *pFrm,
												sal_Bool bCreate = sal_True );
	::com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessible> GetContext(
									 			const SwFrm *pFrm,
												sal_Bool bCreate = sal_True );

	::vos::ORef < ::accessibility::AccessibleShape > GetContextImpl(
										const SdrObject *pObj,
										SwAccessibleContext *pParentImpl,
										sal_Bool bCreate = sal_True );
	::com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessible> GetContext(
										const SdrObject *pObj,
										SwAccessibleContext *pParentImpl,
										sal_Bool bCreate = sal_True );

    inline ViewShell* GetShell() const
    {
        return mpVSh;
    }
	//IAccessibility2 Implementation 2009-----
	sal_Bool IsInSameLevel(const SdrObject* pObj, const SwFEShell* pFESh);
	void AddShapeContext(const SdrObject *pObj,
							 ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > xAccShape);
	
	void AddGroupContext(const SdrObject *pParentObj,
					::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > xAccParent);
	void RemoveGroupContext(const SdrObject *pParentObj, ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > xAccParent);
	//-----IAccessibility2 Implementation 2009

    const SwRect& GetVisArea() const;

    /** get size of a dedicated preview page

        OD 15.01.2003 #103492#
        complete re-factoring of previous method due to new page/print preview
        functionality.

        @author OD

        @param _nPrevwPageNum
        input parameter - physical page number of page visible in the page preview

        @return an object of class <Size>
    */
    Size GetPreViewPageSize( sal_uInt16 _nPrevwPageNum ) const;

	void RemoveContext( const SwFrm *pFrm );
	void RemoveContext( const SdrObject *pObj );

	// Dispose frame and its children if bRecursive is set
    void Dispose( const SwFrm* pFrm,
                  const SdrObject* pObj,
                  Window* pWindow,
                  sal_Bool bRecursive = sal_False );

    void InvalidatePosOrSize( const SwFrm* pFrm,
                              const SdrObject* pObj,
                              Window* pWindow,
							  const SwRect& rOldFrm );

	void InvalidateContent( const SwFrm *pFrm );

    // --> OD 2009-01-06 #i88069#
    void InvalidateAttr( const SwTxtFrm& rTxtFrm );
    // <--

	void InvalidateCursorPosition( const SwFrm *pFrm );
	void InvalidateFocus();
	//IAccessibility2 Implementation 2009-----
	void FirePageChangeEvent(sal_uInt16 nOldPage, sal_uInt16 nNewPage);
	void FireSectionChangeEvent(sal_uInt16 nOldSection, sal_uInt16 nNewSection);
	void FireColumnChangeEvent(sal_uInt16 nOldColumn, sal_uInt16 nNewColumn);
	//-----IAccessibility2 Implementation 2009
	void SetCursorContext(
		const ::vos::ORef < SwAccessibleContext >& rCursorContext );

	// Invalidate state of whole tree. If an action is open, this call
	// is processed when the last action ends.
    // --> OD 2005-12-12 #i27301# - use new type definition for <_nStates>
    void InvalidateStates( tAccessibleStates _nStates,
                           const SwFrm* _pFrm = 0 );

    void InvalidateRelationSet( const SwFrm* pMaster, const SwFrm* pFollow );

    /** invalidation CONTENT_FLOWS_FROM/_TO relation of a paragraph

        OD 2005-12-01 #i27138#

        @author OD

        @param _rTxtFrm
        input parameter - reference to paragraph, whose CONTENT_FLOWS_FROM/_TO
        has to be invalidated.

        @param _bFrom
        input parameter - boolean indicating, if relation CONTENT_FLOWS_FROM
        (value <true>) or CONTENT_FLOWS_TO (value <false>) has to be invalidated.
    */
    void InvalidateParaFlowRelation( const SwTxtFrm& _rTxtFrm,
                                     const bool _bFrom );

    /** invalidation of text selection of a paragraph

        OD 2005-12-12 #i27301#

        @author OD
    */
    void InvalidateParaTextSelection( const SwTxtFrm& _rTxtFrm );

    /** invalidation of text selection of all paragraphs

        OD 2005-12-13 #i27301#

        @author OD
    */
    void InvalidateTextSelectionOfAllParas();

    sal_Int32 GetChildIndex( const SwFrm& rParentFrm,
                             Window& rChild ) const;

    // update preview data (and fire events if necessary)
    // OD 15.01.2003 #103492# - complete re-factoring of method due to new
    // page/print preview functionality.
    void UpdatePreview( const std::vector<PrevwPage*>& _rPrevwPages,
                        const Fraction&  _rScale,
                        const SwPageFrm* _pSelectedPageFrm,
                        const Size&      _rPrevwWinSize );

    void InvalidatePreViewSelection( sal_uInt16 nSelPage );
	sal_Bool IsPageSelected( const SwPageFrm *pPageFrm ) const;

	void FireEvents();


	// IAccessibleViewForwarder

	virtual sal_Bool IsValid() const;
    virtual Rectangle GetVisibleArea() const;
    virtual Point LogicToPixel (const Point& rPoint) const;
    virtual Size LogicToPixel (const Size& rSize) const;
    virtual Point PixelToLogic (const Point& rPoint) const;
    virtual Size PixelToLogic (const Size& rSize) const;

	// IAccessibleParent
    virtual sal_Bool ReplaceChild (
        ::accessibility::AccessibleShape* pCurrentChild,
		const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _rxShape,
		const long _nIndex,
		const ::accessibility::AccessibleShapeTreeInfo& _rShapeTreeInfo
	)	throw (::com::sun::star::uno::RuntimeException);
	//IAccessibility2 Implementation 2009-----
	virtual ::accessibility::AccessibleControlShape* GetAccControlShapeFromModel
		(::com::sun::star::beans::XPropertySet* pSet) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >   GetAccessibleCaption (
		const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xShape)
    throw (::com::sun::star::uno::RuntimeException);
	//-----IAccessibility2 Implementation 2009

    // additional Core/Pixel conversions for internal use; also works
    // for preview
    Point PixelToCore (const Point& rPoint) const;
    Rectangle CoreToPixel (const Rectangle& rRect) const;

private:
    /** get mapping mode for LogicToPixel and PixelToLogic conversions

        OD 15.01.2003 #103492#
        Replacement method <PreviewAdjust(..)> by new method <GetMapMode>.
        Method returns mapping mode of current output device and adjusts it,
        if the shell is in page/print preview.
        Necessary, because <PreviewAdjust(..)> changes mapping mode at current
        output device for mapping logic document positions to page preview window
        positions and vice versa and doesn't take care to recover its changes.

        @author OD

        @param _rPoint
        input parameter - constant reference to point to determine the mapping
        mode adjustments for page/print preview.

        @param _orMapMode
        output parameter - reference to the mapping mode, which is determined
        by the method
    */
    void GetMapMode( const Point& _rPoint,
                     MapMode&     _orMapMode ) const;
//IAccessibility2 Implementation 2009-----
public:
	virtual sal_Bool IsDocumentSelAll();

	::com::sun::star::uno::WeakReference < ::com::sun::star::accessibility::XAccessible > 
		GetCursorContext() const { return mxCursorContext; }

    //Para Container for InvalidateCursorPosition
    typedef std::set< SwAccessibleParagraph* >  SET_PARA;
    SET_PARA m_setParaAdd;
    SET_PARA m_setParaRemove;
    //-----IAccessibility2 Implementation 2009
};
#endif
