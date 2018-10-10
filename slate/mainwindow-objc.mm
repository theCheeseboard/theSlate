/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMacNativeWidget>
#import <AppKit/AppKit.h>

@interface TouchBarProvider: NSResponder <NSTouchBarDelegate, NSApplicationDelegate, NSWindowDelegate>

@property (strong) NSCustomTouchBarItem *saveItem;
@property (strong) NSCustomTouchBarItem *newItem;
@property (strong) NSCustomTouchBarItem *closeItem;
@property (strong) NSCustomTouchBarItem *topbar1Item;
@property (strong) NSCustomTouchBarItem *topbar2Item;
@property (strong) NSButton *saveButton;
@property (strong) NSButton *newButton;
@property (strong) NSButton *closeButton;
@property (strong) NSButton *topbar1Button;
@property (strong) NSButton *topbar2Button;

@property (strong) NSObject *qtDelegate;
@property Ui::MainWindow *mainWindowUi;
@property TopNotification* currentTopNotification;

@end

// Create identifiers for button items.
static NSTouchBarItemIdentifier saveIdentifier = @"org.thesuite.theslate.savebutton";
static NSTouchBarItemIdentifier newIdentifier = @"com.thesuite.theslate.newbutton";
static NSTouchBarItemIdentifier closeIdentifier = @"com.thesuite.theslate.closebutton";
static NSTouchBarItemIdentifier topbar1Identifier = @"com.thesuite.theslate.topbar1";
static NSTouchBarItemIdentifier topbar2Identifier = @"com.thesuite.theslate.topbar2";

@implementation TouchBarProvider

- (id)init:(Ui::MainWindow*)ui {
    if (self = [super init]) {
        //Set main window UI to call touch bar handlers
        self.mainWindowUi = ui;
    }

    return self;
}

- (NSTouchBar *)makeTouchBar
{
    // Create the touch bar with this instance as its delegate
    NSTouchBar *bar = [[NSTouchBar alloc] init];
    bar.delegate = self;

    // Add touch bar items: first, the very important emoji picker, followed
    // by two buttons. Note that no further handling of the emoji picker
    // is needed (emojii are automatically routed to any active text edit). Button
    // actions handlers are set up in makeItemForIdentifier below.
    bar.defaultItemIdentifiers = @[NSTouchBarItemIdentifierCharacterPicker, newIdentifier, saveIdentifier, NSTouchBarItemIdentifierFlexibleSpace, topbar1Identifier, topbar2Identifier, closeIdentifier];
    bar.customizationRequiredItemIdentifiers = @[topbar1Identifier, topbar2Identifier];
    bar.customizationAllowedItemIdentifiers = @[NSTouchBarItemIdentifierCharacterPicker, newIdentifier, saveIdentifier, closeIdentifier, NSTouchBarItemIdentifierFlexibleSpace];
    [bar setCustomizationIdentifier:@"org.thesuite.theslate.touchbar"];

    return bar;
}

- (void)setTopNotification:(TopNotification*)topNotification {
    self.currentTopNotification = topNotification;

    if (topNotification == nullptr) {
        [self.topbar1Button setHidden:YES];
        [self.topbar2Button setHidden:YES];
    } else {
        if (topNotification->firstButton() == nullptr) {
            [self.topbar1Button setHidden:YES];
        } else {
            [self.topbar1Button setTitle:topNotification->firstButton()->text().toNSString()];
            [self.topbar1Button setHidden:NO];
        }

        if (topNotification->secondButton() == nullptr) {
            [self.topbar2Button setHidden:YES];
        } else {
            [self.topbar2Button setTitle:topNotification->secondButton()->text().toNSString()];
            [self.topbar2Button setHidden:NO];
        }
    }
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    Q_UNUSED(touchBar);

    // Create touch bar items as NSCustomTouchBarItems which can contain any NSView.
    if ([identifier isEqualToString:saveIdentifier]) {
        QString title = QApplication::translate("MainWindow", "Save");
        self.saveItem = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
        [self.saveItem setCustomizationLabel:QApplication::translate("MainWindow", "Save").toNSString()];
        self.saveButton = [[NSButton buttonWithTitle:title.toNSString() target:self
                                          action:@selector(saveClicked)] autorelease];
        self.saveItem.view =  self.saveButton;

        QObject::connect(self.mainWindowUi->actionSave, &QAction::changed, [=] {
            if (self.mainWindowUi->actionSave->isEnabled()) {
                [self.saveButton setEnabled: YES];
            } else {
                [self.saveButton setEnabled: NO];
            }
        });

        return self.saveItem;
    } else if ([identifier isEqualToString:newIdentifier]) {
        QString title = QApplication::translate("MainWindow", "New");
        self.newItem = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
        [self.newItem setCustomizationLabel:QApplication::translate("MainWindow", "New").toNSString()];
        self.newButton = [[NSButton buttonWithTitle:title.toNSString() target:self
                                          action:@selector(newClicked)] autorelease];
        self.newItem.view =  self.newButton;
        return self.newItem;
    } else if ([identifier isEqualToString:closeIdentifier]) {
        QString title = QApplication::translate("MainWindow", "Close");
        self.closeItem = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
        [self.closeItem setCustomizationLabel:QApplication::translate("MainWindow", "Close").toNSString()];
        self.closeButton = [[NSButton buttonWithTitle:title.toNSString() target:self
                                          action:@selector(closeClicked)] autorelease];
        self.closeItem.view =  self.closeButton;

        QObject::connect(self.mainWindowUi->actionClose, &QAction::changed, [=] {
            if (self.mainWindowUi->actionClose->isEnabled()) {
                [self.closeButton setEnabled: YES];
            } else {
                [self.closeButton setEnabled: NO];
            }
        });

        return self.closeItem;
    } else if ([identifier isEqualToString:topbar1Identifier]) {
        self.topbar1Item = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
        self.topbar1Button = [[NSButton buttonWithTitle:@"" target:self
                                          action:@selector(topbar1Clicked)] autorelease];
        self.topbar1Item.view =  self.topbar1Button;
        [self.topbar1Button setHidden:YES];

        return self.topbar1Item;
    } else if ([identifier isEqualToString:topbar2Identifier]) {
        self.topbar2Item = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
        self.topbar2Button = [[NSButton buttonWithTitle:@"" target:self
                                          action:@selector(topbar2Clicked)] autorelease];
        self.topbar2Item.view =  self.topbar2Button;
        [self.topbar2Button setHidden:YES];

        return self.topbar2Item;
    }
   return nil;
}

- (void)installAsDelegateForWindow:(NSWindow *)window
{
    _qtDelegate = window.delegate; // Save current delegate for forwarding
    window.delegate = self;
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
    // We want to forward to the qt delegate. Respond to selectors it
    // responds to in addition to selectors this instance resonds to.
    return [_qtDelegate respondsToSelector:aSelector] || [super respondsToSelector:aSelector];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    // Forward to the existing delegate. This function is only called for selectors
    // this instance does not responds to, which means that the qt delegate
    // must respond to it (due to the respondsToSelector implementation above).
    [anInvocation invokeWithTarget:_qtDelegate];
}

- (void)saveClicked
{
    //Save button on touch bar was clicked
    //Forward signal to the save action on the main window
    self.mainWindowUi->actionSave->trigger();
}

- (void)newClicked
{
    //New button on touch bar was clicked
    //Forward signal to the new action on the main window
    self.mainWindowUi->actionNew->trigger();
}

- (void)closeClicked
{
    //Close button on touch bar was clicked
    //Forward signal to the new action on the main window
    self.mainWindowUi->actionClose->trigger();
}

- (void)topbar1Clicked
{
    //Top Notification button 1 on touch bar was clicked
    //Forward signal to the button if available
    if (self.currentTopNotification != nullptr) {
        if (self.currentTopNotification->firstButton() != nullptr) {
            self.currentTopNotification->firstButton()->click();
        }
    }
}

- (void)topbar2Clicked
{
    //Top Notification button 2 on touch bar was clicked
    //Forward signal to the button if available

    if (self.currentTopNotification != nullptr) {
        if (self.currentTopNotification->secondButton() != nullptr) {
            self.currentTopNotification->secondButton()->click();
        }
    }
}

- (NSApplicationPresentationOptions)window:(NSWindow *)window willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions)proposedOptions {
    //On an unrelated note, set full screen window properties
    return (NSApplicationPresentationFullScreen | NSApplicationPresentationHideDock | NSApplicationPresentationAutoHideMenuBar | NSApplicationPresentationAutoHideToolbar);
}
@end

void MainWindow::setupMacOS() {
    //Install TouchBarProvider as window delegate
    NSView *view = reinterpret_cast<NSView *>(this->winId());
    TouchBarProvider *touchBarProvider = [[TouchBarProvider alloc] init:ui];
    [touchBarProvider installAsDelegateForWindow:view.window];

    connect(this, &MainWindow::changeTouchBarTopNotification, [=](TopNotification* notification) {
        [touchBarProvider setTopNotification:notification];
    });
}

void MainWindow::updateTouchBar() {
    //Invalidate Touch Bar
    NSView *view = reinterpret_cast<NSView *>(this->winId());
    view.window.touchBar = nil;
}

void setToolbarItemWidget(QMacToolBarItem* item, QWidget* widget) {
    QMacNativeWidget* nativeWidget = new QMacNativeWidget();
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->addWidget(widget);
    widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    nativeWidget->setLayout(layout);
    nativeWidget->setFixedHeight(100);

    NSView* nativeView = reinterpret_cast<NSView*>(nativeWidget->winId());
    [nativeView setAutoresizingMask:NSViewWidthSizable];
    [item->nativeToolBarItem() setView:nativeView];
    //nativeWidget->show();
}
