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
#include "precompiled_vcl.hxx"

#include "aqua/salinst.h"
#include "aqua/saldata.hxx"
#include "aqua/salframe.h"
#include "aqua/salmenu.h"
#include "aqua/salnsmenu.h"

#include "vcl/window.hxx"
 
@implementation SalNSMenu
-(id)initWithMenu: (AquaSalMenu*)pMenu
{
    mpMenu = pMenu;
    return [super initWithTitle: [NSString string]];
}

-(void)menuNeedsUpdate: (NSMenu*)pMenu
{
    (void)pMenu;
    YIELD_GUARD;
    
    if( mpMenu )
    {
        const AquaSalFrame* pFrame = mpMenu->getFrame();
        if( pFrame && AquaSalFrame::isAlive( pFrame ) )
        {
            SalMenuEvent aMenuEvt;
            aMenuEvt.mnId   = 0;
            aMenuEvt.mpMenu = mpMenu->mpVCLMenu;
            if( aMenuEvt.mpMenu )
            {
                pFrame->CallCallback(SALEVENT_MENUACTIVATE, &aMenuEvt);
                pFrame->CallCallback(SALEVENT_MENUDEACTIVATE, &aMenuEvt);
            }
            else
                DBG_ERROR( "unconnected menu" );
        }
    }
}

-(void)setSalMenu: (AquaSalMenu*)pMenu
{
    mpMenu = pMenu;
}
@end

@implementation SalNSMenuItem
-(id)initWithMenuItem: (AquaSalMenuItem*)pMenuItem
{
    mpMenuItem = pMenuItem;
    id ret = [super initWithTitle: [NSString string]
                    action: @selector(menuItemTriggered:)
                    keyEquivalent: [NSString string]];
    [ret setTarget: self];
    return ret;
}
-(void)menuItemTriggered: (id)aSender
{
    (void)aSender;
    YIELD_GUARD;
    
    const AquaSalFrame* pFrame = mpMenuItem->mpParentMenu ? mpMenuItem->mpParentMenu->getFrame() : NULL;
    if( pFrame && AquaSalFrame::isAlive( pFrame ) && ! pFrame->GetWindow()->IsInModalMode() )
    {
        SalMenuEvent aMenuEvt( mpMenuItem->mnId, mpMenuItem->mpVCLMenu );
        pFrame->CallCallback(SALEVENT_MENUCOMMAND, &aMenuEvt);
    }
    else if( mpMenuItem->mpVCLMenu )
    {
        // if an item from submenu was selected. the corresponding Window does not exist because
        // we use native popup menus, so we have to set the selected menuitem directly
        // incidentally this of course works for top level popup menus, too
        PopupMenu * pPopupMenu = dynamic_cast<PopupMenu *>(mpMenuItem->mpVCLMenu);
        if( pPopupMenu )
        {
            // FIXME: revise this ugly code
            
            // select handlers in vcl are dispatch on the original menu
            // if not consumed by the select handler of the current menu
            // however since only the starting menu ever came into Execute
            // the hierarchy is not build up. Workaround this by getting
            // the menu it should have been
            
            // get started from hierarchy in vcl menus
            AquaSalMenu* pParentMenu = mpMenuItem->mpParentMenu;
            Menu* pCurMenu = mpMenuItem->mpVCLMenu;
            while( pParentMenu && pParentMenu->mpVCLMenu )
            {
                pCurMenu = pParentMenu->mpVCLMenu;
                pParentMenu = pParentMenu->mpParentSalMenu;
            }
            
            pPopupMenu->SetSelectedEntry( mpMenuItem->mnId );
            pPopupMenu->ImplSelectWithStart( pCurMenu );
        }
        else
            DBG_ERROR( "menubar item without frame !" );
    }
}
@end

@implementation OOStatusItemView
-(void)drawRect: (NSRect)aRect
{
    NSGraphicsContext* pContext = [NSGraphicsContext currentContext];
    [pContext saveGraphicsState];
    [SalData::getStatusItem() drawStatusBarBackgroundInRect: aRect withHighlight: NO];
    if( AquaSalMenu::pCurrentMenuBar )
    {
        const std::vector< AquaSalMenu::MenuBarButtonEntry >& rButtons( AquaSalMenu::pCurrentMenuBar->getButtons() );
        NSRect aFrame = [self frame];
        NSRect aImgRect = { { 2, 0 }, { 0, 0 } };
        for( size_t i = 0; i < rButtons.size(); ++i )
        {
            const Size aPixSize = rButtons[i].maButton.maImage.GetSizePixel();
            const NSRect aFromRect = NSMakeRect( 0, 0, aPixSize.Width(), aPixSize.Height());
            aImgRect.origin.y = floor((aFrame.size.height - aFromRect.size.height)/2);
            aImgRect.size = aFromRect.size;
            if( rButtons[i].mpNSImage )
                [rButtons[i].mpNSImage drawInRect: aImgRect fromRect: aFromRect operation: NSCompositeSourceOver fraction: 1.0];
            aImgRect.origin.x += aFromRect.size.width + 2;
        }
    }
    [pContext restoreGraphicsState];
}

-(void)mouseUp: (NSEvent *)pEvent
{
    /* check if button goes up inside one of our status buttons */
    if( AquaSalMenu::pCurrentMenuBar )
    {
        const std::vector< AquaSalMenu::MenuBarButtonEntry >& rButtons( AquaSalMenu::pCurrentMenuBar->getButtons() );
        NSRect aFrame = [self frame];
        NSRect aImgRect = { { 2, 0 }, { 0, 0 } };
        NSPoint aMousePt = [pEvent locationInWindow];
        for( size_t i = 0; i < rButtons.size(); ++i )
        {
            const Size aPixSize = rButtons[i].maButton.maImage.GetSizePixel();
            const NSRect aFromRect = NSMakeRect( 0, 0, aPixSize.Width(), aPixSize.Height());
            aImgRect.origin.y = (aFrame.size.height - aFromRect.size.height)/2;
            aImgRect.size = aFromRect.size;
            if( aMousePt.x >= aImgRect.origin.x && aMousePt.x <= (aImgRect.origin.x+aImgRect.size.width) &&
                aMousePt.y >= aImgRect.origin.y && aMousePt.y <= (aImgRect.origin.y+aImgRect.size.height) )
            {
                if( AquaSalMenu::pCurrentMenuBar->mpFrame && AquaSalFrame::isAlive( AquaSalMenu::pCurrentMenuBar->mpFrame ) )
                {
                    SalMenuEvent aMenuEvt( rButtons[i].maButton.mnId, AquaSalMenu::pCurrentMenuBar->mpVCLMenu );
                    AquaSalMenu::pCurrentMenuBar->mpFrame->CallCallback(SALEVENT_MENUBUTTONCOMMAND, &aMenuEvt);
                }
                return;
            }
            
            aImgRect.origin.x += aFromRect.size.width + 2;
        }
    }
}

-(void)layout
{
    NSStatusBar* pStatBar = [NSStatusBar systemStatusBar];
    NSSize aSize = { 0, [pStatBar thickness] };
    [self removeAllToolTips];
    if( AquaSalMenu::pCurrentMenuBar )
    {
        const std::vector< AquaSalMenu::MenuBarButtonEntry >& rButtons( AquaSalMenu::pCurrentMenuBar->getButtons() );
        if( ! rButtons.empty() )
        {
            aSize.width = 2;
            for( size_t i = 0; i < rButtons.size(); ++i )
            {
                const Size aPixSize = rButtons[i].maButton.maImage.GetSizePixel();
                const int nY = floor( aSize.height - aPixSize.Height() ) / 2;
                NSRect aImgRect = NSMakeRect( aSize.width, nY, aPixSize.Width(), aPixSize.Height());
                if( rButtons[i].mpToolTipString )
                    [self addToolTipRect: aImgRect owner: rButtons[i].mpToolTipString userData: NULL];
                aSize.width += 2 + aImgRect.size.width;
            }
        }
    }
    [self setFrameSize: aSize];
}
@end


