//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include "internal.hpp"

#if defined(_GRWL_COCOA)

    #include <cfloat>
    #include <cstring>

    // HACK: This enum value is missing from framework headers on OS X 10.11 despite
    //       having been (according to documentation) added in Mac OS X 10.7
    #define NSWindowCollectionBehaviorFullScreenNone (1 << 9)

// Returns whether the cursor is in the content area of the specified window
//
static GRWLbool cursorInContentArea(_GRWLwindow* window)
{
    const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];
    return [window->ns.view mouse:pos inRect:[window->ns.view frame]];
}

// Hides the cursor if not already hidden
//
static void hideCursor(_GRWLwindow* window)
{
    if (!_grwl.ns.cursorHidden)
    {
        [NSCursor hide];
        _grwl.ns.cursorHidden = GRWL_TRUE;
    }
}

// Shows the cursor if not already shown
//
static void showCursor(_GRWLwindow* window)
{
    if (_grwl.ns.cursorHidden)
    {
        [NSCursor unhide];
        _grwl.ns.cursorHidden = GRWL_FALSE;
    }
}

// Updates the cursor image according to its cursor mode
//
static void updateCursorImage(_GRWLwindow* window)
{
    if (window->cursorMode == GRWL_CURSOR_NORMAL)
    {
        showCursor(window);

        if (window->cursor)
        {
            [(NSCursor*)window->cursor->ns.object set];
        }
        else
        {
            [[NSCursor arrowCursor] set];
        }
    }
    else
    {
        hideCursor(window);
    }
}

// Apply chosen cursor mode to a focused window
//
static void updateCursorMode(_GRWLwindow* window)
{
    if (window->cursorMode == GRWL_CURSOR_DISABLED)
    {
        _grwl.ns.disabledCursorWindow = window;
        _grwlGetCursorPosCocoa(window, &_grwl.ns.restoreCursorPosX, &_grwl.ns.restoreCursorPosY);
        _grwlCenterCursorInContentArea(window);
        CGAssociateMouseAndMouseCursorPosition(false);
    }
    else if (_grwl.ns.disabledCursorWindow == window)
    {
        _grwl.ns.disabledCursorWindow = NULL;
        _grwlSetCursorPosCocoa(window, _grwl.ns.restoreCursorPosX, _grwl.ns.restoreCursorPosY);
        // NOTE: The matching CGAssociateMouseAndMouseCursorPosition call is
        //       made in _grwlSetCursorPosCocoa as part of a workaround
    }

    if (cursorInContentArea(window))
    {
        updateCursorImage(window);
    }
}

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_GRWLwindow* window)
{
    _grwlSetVideoModeCocoa(window->monitor, &window->videoMode);
    const CGRect bounds = CGDisplayBounds(window->monitor->ns.displayID);
    const NSRect frame = NSMakeRect(bounds.origin.x, _grwlTransformYCocoa(bounds.origin.y + bounds.size.height - 1),
                                    bounds.size.width, bounds.size.height);

    [window->ns.object setFrame:frame display:YES];

    _grwlInputMonitorWindow(window->monitor, window);
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_GRWLwindow* window)
{
    if (window->monitor->window != window)
    {
        return;
    }

    _grwlInputMonitorWindow(window->monitor, NULL);
    _grwlRestoreVideoModeCocoa(window->monitor);
}

// Translates macOS key modifiers into GRWL ones
//
static int translateFlags(NSUInteger flags)
{
    int mods = 0;

    if (flags & NSEventModifierFlagShift)
    {
        mods |= GRWL_MOD_SHIFT;
    }
    if (flags & NSEventModifierFlagControl)
    {
        mods |= GRWL_MOD_CONTROL;
    }
    if (flags & NSEventModifierFlagOption)
    {
        mods |= GRWL_MOD_ALT;
    }
    if (flags & NSEventModifierFlagCommand)
    {
        mods |= GRWL_MOD_SUPER;
    }
    if (flags & NSEventModifierFlagCapsLock)
    {
        mods |= GRWL_MOD_CAPS_LOCK;
    }

    return mods;
}

// Translates a macOS keycode to a GRWL keycode
//
static int translateKey(unsigned int key)
{
    if (key >= sizeof(_grwl.ns.keycodes) / sizeof(_grwl.ns.keycodes[0]))
    {
        return GRWL_KEY_UNKNOWN;
    }

    return _grwl.ns.keycodes[key];
}

// Translate a GRWL keycode to a Cocoa modifier flag
//
static NSUInteger translateKeyToModifierFlag(int key)
{
    switch (key)
    {
        case GRWL_KEY_LEFT_SHIFT:
        case GRWL_KEY_RIGHT_SHIFT:
            return NSEventModifierFlagShift;
        case GRWL_KEY_LEFT_CONTROL:
        case GRWL_KEY_RIGHT_CONTROL:
            return NSEventModifierFlagControl;
        case GRWL_KEY_LEFT_ALT:
        case GRWL_KEY_RIGHT_ALT:
            return NSEventModifierFlagOption;
        case GRWL_KEY_LEFT_SUPER:
        case GRWL_KEY_RIGHT_SUPER:
            return NSEventModifierFlagCommand;
        case GRWL_KEY_CAPS_LOCK:
            return NSEventModifierFlagCapsLock;
    }

    return 0;
}

// Defines a constant for empty ranges in NSTextInputClient
//
static const NSRange kEmptyRange = { NSNotFound, 0 };

static NSProgressIndicator* createProgressIndicator(const NSDockTile* dockTile)
{
    NSView* contentView = [dockTile contentView];

    NSProgressIndicator* indicator =
        [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(0.0f, 0.0f, contentView.frame.size.width, 15.0f)];

    [indicator setStyle:NSProgressIndicatorStyleBar];

    if (@available(macOS 11.0, *))
    {
        [indicator setControlSize:NSControlSizeLarge];
    }

    [indicator setMinValue:0.0f];
    [indicator setMaxValue:1.0f];

    [indicator sizeToFit];

    [contentView addSubview:indicator];

    _grwl.ns.dockProgressIndicator.view = indicator;

    return indicator;
}

static void setDockProgressIndicator(int progressState, double value)
{
    NSProgressIndicator* indicator = _grwl.ns.dockProgressIndicator.view;

    NSDockTile* dockTile = [[NSApplication sharedApplication] dockTile];

    if (indicator == nil)
    {
        if ([dockTile contentView] == nil)
        {
            NSImageView* iconView = [[NSImageView alloc] init];
            [iconView setImage:[[NSApplication sharedApplication] applicationIconImage]];
            [dockTile setContentView:iconView];
            [iconView release];
        }

        indicator = createProgressIndicator(dockTile);
    }

    // ### Switching from INDETERMINATE to NORMAL, PAUSED or ERROR requires 2 invocations in different frames.
    // In MacOS 12 (and probably other versions), an indeterminate progress bar is rendered as a normal bar
    // with 0.0 progress. So when calling [progressIndicator setIndeterminate:YES], the indicator actually
    // sets its doubleValue to 0.0.
    // The bug is caused by NSProgressIndicator not immediately updating its value when it's increasing.
    // This code illustrates the exact same problem, but this time from NORMAL, PAUSED and ERROR to INDETERMINATE:
    //
    // if (progressState == GRWL_PROGRESS_INDICATOR_INDETERMINATE)
    //     [progressIndicator setDoubleValue:0.75];
    // else
    //     [progressIndicator setDoubleValue:0.25];
    //
    // This is likely a bug in Cocoa.
    //
    // ### Progress increments are delayed
    // What this also means, is that each time the progress increments, the bar's progress will be 1 frame delayed,
    // and only updated once a higher or similar value is again set the next frame.

    // Workaround for the aforementioned issues. If there's any versions of MacOS where
    // this issue is not present, this should be ommitted in those versions.
    if ([indicator isIndeterminate] || [indicator doubleValue] < value)
    {
        [indicator removeFromSuperview];
        [indicator release];
        indicator = createProgressIndicator(dockTile);
    }

    [indicator setIndeterminate:progressState == GRWL_PROGRESS_INDICATOR_INDETERMINATE];
    [indicator setHidden:progressState == GRWL_PROGRESS_INDICATOR_DISABLED];
    [indicator setDoubleValue:value];

    [dockTile display];
}

//------------------------------------------------------------------------
// Delegate for window related notifications
//------------------------------------------------------------------------

@interface GRWLWindowDelegate: NSObject
{
    _GRWLwindow* window;
}

- (instancetype)initWithGlfwWindow:(_GRWLwindow*)initWindow;

@end

@implementation GRWLWindowDelegate

- (instancetype)initWithGlfwWindow:(_GRWLwindow*)initWindow
{
    self = [super init];
    if (self != nil)
    {
        window = initWindow;
    }

    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    _grwlInputWindowCloseRequest(window);
    return NO;
}

- (void)windowDidResize:(NSNotification*)notification
{
    if (window->context.source == GRWL_NATIVE_CONTEXT_API)
    {
        [window->context.nsgl.object update];
    }

    if (_grwl.ns.disabledCursorWindow == window)
    {
        _grwlCenterCursorInContentArea(window);
    }

    const int maximized = [window->ns.object isZoomed];
    if (window->ns.maximized != maximized)
    {
        window->ns.maximized = maximized;
        _grwlInputWindowMaximize(window, maximized);
    }

    const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];

    if (fbRect.size.width != window->ns.fbWidth || fbRect.size.height != window->ns.fbHeight)
    {
        window->ns.fbWidth = fbRect.size.width;
        window->ns.fbHeight = fbRect.size.height;
        _grwlInputFramebufferSize(window, fbRect.size.width, fbRect.size.height);
    }

    if (contentRect.size.width != window->ns.width || contentRect.size.height != window->ns.height)
    {
        window->ns.width = contentRect.size.width;
        window->ns.height = contentRect.size.height;
        _grwlInputWindowSize(window, contentRect.size.width, contentRect.size.height);
    }
}

- (void)windowDidMove:(NSNotification*)notification
{
    if (window->context.source == GRWL_NATIVE_CONTEXT_API)
    {
        [window->context.nsgl.object update];
    }

    if (_grwl.ns.disabledCursorWindow == window)
    {
        _grwlCenterCursorInContentArea(window);
    }

    int x, y;
    _grwlGetWindowPosCocoa(window, &x, &y);
    _grwlInputWindowPos(window, x, y);
}

- (void)windowDidMiniaturize:(NSNotification*)notification
{
    if (window->monitor)
    {
        releaseMonitor(window);
    }

    _grwlInputWindowIconify(window, GRWL_TRUE);
}

- (void)windowDidDeminiaturize:(NSNotification*)notification
{
    if (window->monitor)
    {
        acquireMonitor(window);
    }

    _grwlInputWindowIconify(window, GRWL_FALSE);
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
    if (_grwl.ns.disabledCursorWindow == window)
    {
        _grwlCenterCursorInContentArea(window);
    }

    _grwlInputWindowFocus(window, GRWL_TRUE);
    updateCursorMode(window);
}

- (void)windowDidResignKey:(NSNotification*)notification
{
    if (window->monitor && window->autoIconify)
    {
        _grwlIconifyWindowCocoa(window);
    }

    _grwlInputWindowFocus(window, GRWL_FALSE);
}

- (void)windowDidChangeOcclusionState:(NSNotification*)notification
{
    if ([window->ns.object occlusionState] & NSWindowOcclusionStateVisible)
    {
        window->ns.occluded = GRWL_FALSE;
    }
    else
    {
        window->ns.occluded = GRWL_TRUE;
    }
}

- (void)imeStatusChangeNotified:(NSNotification*)notification
{
    _grwlInputIMEStatus(window);
}

@end

//------------------------------------------------------------------------
// Content view class for the GRWL window
//------------------------------------------------------------------------

@interface GRWLContentView: NSView <NSTextInputClient>
{
    _GRWLwindow* window;
    NSTrackingArea* trackingArea;
    NSMutableAttributedString* markedText;
}

- (instancetype)initWithGlfwWindow:(_GRWLwindow*)initWindow;

@end

@implementation GRWLContentView

- (instancetype)initWithGlfwWindow:(_GRWLwindow*)initWindow
{
    self = [super init];
    if (self != nil)
    {
        window = initWindow;
        trackingArea = nil;
        markedText = [[NSMutableAttributedString alloc] init];

        [self updateTrackingAreas];
        [self registerForDraggedTypes:@[ NSPasteboardTypeURL ]];
    }

    return self;
}

- (void)dealloc
{
    [trackingArea release];
    [markedText release];
    [super dealloc];
}

- (BOOL)isOpaque
{
    return [window->ns.object isOpaque];
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)wantsUpdateLayer
{
    return YES;
}

- (void)updateLayer
{
    if (window->context.source == GRWL_NATIVE_CONTEXT_API)
    {
        [window->context.nsgl.object update];
    }

    _grwlInputWindowDamage(window);
}

- (void)cursorUpdate:(NSEvent*)event
{
    updateCursorImage(window);
}

- (BOOL)acceptsFirstMouse:(NSEvent*)event
{
    return YES;
}

- (void)mouseDown:(NSEvent*)event
{
    _grwlInputMouseClick(window, GRWL_MOUSE_BUTTON_LEFT, GRWL_PRESS, translateFlags([event modifierFlags]));
}

- (void)mouseDragged:(NSEvent*)event
{
    [self mouseMoved:event];
}

- (void)mouseUp:(NSEvent*)event
{
    _grwlInputMouseClick(window, GRWL_MOUSE_BUTTON_LEFT, GRWL_RELEASE, translateFlags([event modifierFlags]));
}

- (void)mouseMoved:(NSEvent*)event
{
    if (window->cursorMode == GRWL_CURSOR_DISABLED)
    {
        const double dx = [event deltaX] - window->ns.cursorWarpDeltaX;
        const double dy = [event deltaY] - window->ns.cursorWarpDeltaY;

        _grwlInputCursorPos(window, window->virtualCursorPosX + dx, window->virtualCursorPosY + dy);
    }
    else
    {
        const NSRect contentRect = [window->ns.view frame];
        // NOTE: The returned location uses base 0,1 not 0,0
        const NSPoint pos = [event locationInWindow];

        _grwlInputCursorPos(window, pos.x, contentRect.size.height - pos.y);
    }

    window->ns.cursorWarpDeltaX = 0;
    window->ns.cursorWarpDeltaY = 0;
}

- (void)rightMouseDown:(NSEvent*)event
{
    _grwlInputMouseClick(window, GRWL_MOUSE_BUTTON_RIGHT, GRWL_PRESS, translateFlags([event modifierFlags]));
}

- (void)rightMouseDragged:(NSEvent*)event
{
    [self mouseMoved:event];
}

- (void)rightMouseUp:(NSEvent*)event
{
    _grwlInputMouseClick(window, GRWL_MOUSE_BUTTON_RIGHT, GRWL_RELEASE, translateFlags([event modifierFlags]));
}

- (void)otherMouseDown:(NSEvent*)event
{
    _grwlInputMouseClick(window, (int)[event buttonNumber], GRWL_PRESS, translateFlags([event modifierFlags]));
}

- (void)otherMouseDragged:(NSEvent*)event
{
    [self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent*)event
{
    _grwlInputMouseClick(window, (int)[event buttonNumber], GRWL_RELEASE, translateFlags([event modifierFlags]));
}

- (void)mouseExited:(NSEvent*)event
{
    if (window->cursorMode == GRWL_CURSOR_HIDDEN)
    {
        showCursor(window);
    }

    _grwlInputCursorEnter(window, GRWL_FALSE);
}

- (void)mouseEntered:(NSEvent*)event
{
    if (window->cursorMode == GRWL_CURSOR_HIDDEN)
    {
        hideCursor(window);
    }

    _grwlInputCursorEnter(window, GRWL_TRUE);
}

- (void)viewDidChangeBackingProperties
{
    const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];
    const float xscale = fbRect.size.width / contentRect.size.width;
    const float yscale = fbRect.size.height / contentRect.size.height;

    if (xscale != window->ns.xscale || yscale != window->ns.yscale)
    {
        if (window->ns.retina && window->ns.layer)
        {
            [window->ns.layer setContentsScale:[window->ns.object backingScaleFactor]];
        }

        window->ns.xscale = xscale;
        window->ns.yscale = yscale;
        _grwlInputWindowContentScale(window, xscale, yscale);
    }

    if (fbRect.size.width != window->ns.fbWidth || fbRect.size.height != window->ns.fbHeight)
    {
        window->ns.fbWidth = fbRect.size.width;
        window->ns.fbHeight = fbRect.size.height;
        _grwlInputFramebufferSize(window, fbRect.size.width, fbRect.size.height);
    }
}

- (void)drawRect:(NSRect)rect
{
    _grwlInputWindowDamage(window);
}

- (void)updateTrackingAreas
{
    if (trackingArea != nil)
    {
        [self removeTrackingArea:trackingArea];
        [trackingArea release];
    }

    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow |
                                          NSTrackingEnabledDuringMouseDrag | NSTrackingCursorUpdate |
                                          NSTrackingInVisibleRect | NSTrackingAssumeInside;

    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:options owner:self userInfo:nil];

    [self addTrackingArea:trackingArea];
    [super updateTrackingAreas];
}

- (void)keyDown:(NSEvent*)event
{
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags([event modifierFlags]);

    if (![self hasMarkedText])
    {
        _grwlInputKey(window, key, [event keyCode], GRWL_PRESS, mods);
    }

    [self interpretKeyEvents:@[ event ]];
}

- (void)flagsChanged:(NSEvent*)event
{
    int action;
    const unsigned int modifierFlags = [event modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask;
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags(modifierFlags);
    const NSUInteger keyFlag = translateKeyToModifierFlag(key);

    if (keyFlag & modifierFlags)
    {
        if (window->keys[key] == GRWL_PRESS)
        {
            action = GRWL_RELEASE;
        }
        else
        {
            action = GRWL_PRESS;
        }
    }
    else
    {
        action = GRWL_RELEASE;
    }

    _grwlInputKey(window, key, [event keyCode], action, mods);
}

- (void)keyUp:(NSEvent*)event
{
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags([event modifierFlags]);
    _grwlInputKey(window, key, [event keyCode], GRWL_RELEASE, mods);
}

- (void)scrollWheel:(NSEvent*)event
{
    double deltaX = [event scrollingDeltaX];
    double deltaY = [event scrollingDeltaY];

    if ([event hasPreciseScrollingDeltas])
    {
        deltaX *= 0.1;
        deltaY *= 0.1;
    }

    if (fabs(deltaX) > 0.0 || fabs(deltaY) > 0.0)
    {
        _grwlInputScroll(window, deltaX, deltaY);
    }
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    // HACK: We don't know what to say here because we don't know what the
    //       application wants to do with the paths
    return NSDragOperationGeneric;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    const NSRect contentRect = [window->ns.view frame];
    // NOTE: The returned location uses base 0,1 not 0,0
    const NSPoint pos = [sender draggingLocation];
    _grwlInputCursorPos(window, pos.x, contentRect.size.height - pos.y);

    NSPasteboard* pasteboard = [sender draggingPasteboard];
    NSDictionary* options = @{ NSPasteboardURLReadingFileURLsOnlyKey : @YES };
    NSArray* urls = [pasteboard readObjectsForClasses:@[ [NSURL class] ] options:options];
    const NSUInteger count = [urls count];
    if (count)
    {
        char** paths = _grwl_calloc(count, sizeof(char*));

        for (NSUInteger i = 0; i < count; i++)
        {
            paths[i] = _grwl_strdup([urls[i] fileSystemRepresentation]);
        }

        _grwlInputDrop(window, (int)count, (const char**)paths);

        for (NSUInteger i = 0; i < count; i++)
        {
            _grwl_free(paths[i]);
        }
        _grwl_free(paths);
    }

    return YES;
}

- (BOOL)hasMarkedText
{
    return [markedText length] > 0;
}

- (NSRange)markedRange
{
    if ([markedText length] > 0)
    {
        return NSMakeRange(0, [markedText length]);
    }
    else
    {
        return kEmptyRange;
    }
}

- (NSRange)selectedRange
{
    return kEmptyRange;
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
    [markedText release];
    if ([string isKindOfClass:[NSAttributedString class]])
    {
        markedText = [[NSMutableAttributedString alloc] initWithAttributedString:string];
    }
    else
    {
        markedText = [[NSMutableAttributedString alloc] initWithString:string];
    }

    NSString* markedTextString = markedText.string;

    NSUInteger textLen = [markedTextString length];
    _GRWLpreedit* preedit = &window->preedit;
    int textBufferCount = preedit->textBufferCount;
    while (textBufferCount < textLen + 1)
    {
        textBufferCount = textBufferCount == 0 ? 1 : textBufferCount * 2;
    }
    if (textBufferCount != preedit->textBufferCount)
    {
        unsigned int* preeditText = _grwl_realloc(preedit->text, sizeof(unsigned int) * textBufferCount);
        if (preeditText == NULL)
        {
            return;
        }
        preedit->text = preeditText;
        preedit->textBufferCount = textBufferCount;
    }

    // NSString handles text data in UTF16 by default, so we have to convert them
    // to UTF32. Not only the encoding, but also the number of characters and
    // the position of each block.
    int currentBlockIndex = 0;
    int currentBlockLength = 0;
    int currentBlockLocation = 0;
    int focusedBlockIndex = 0;
    NSInteger preeditTextLength = 0;
    NSRange range = NSMakeRange(0, textLen);
    while (range.length)
    {
        uint32_t codepoint = 0;
        NSRange currentBlockRange;
        [markedText attributesAtIndex:range.location effectiveRange:&currentBlockRange];

        if (preedit->blockSizesBufferCount < 1 + currentBlockIndex)
        {
            int blockBufferCount = (preedit->blockSizesBufferCount == 0) ? 1 : preedit->blockSizesBufferCount * 2;
            int* blocks = _grwl_realloc(preedit->blockSizes, sizeof(int) * blockBufferCount);
            if (blocks == NULL)
            {
                return;
            }
            preedit->blockSizes = blocks;
            preedit->blockSizesBufferCount = blockBufferCount;
        }

        if (currentBlockLocation != currentBlockRange.location)
        {
            currentBlockLocation = currentBlockRange.location;
            preedit->blockSizes[currentBlockIndex++] = currentBlockLength;
            currentBlockLength = 0;
            if (selectedRange.location == currentBlockRange.location)
            {
                focusedBlockIndex = currentBlockIndex;
            }
        }

        if ([markedTextString getBytes:&codepoint
                             maxLength:sizeof(codepoint)
                            usedLength:NULL
                              encoding:NSUTF32StringEncoding
                               options:0
                                 range:range
                        remainingRange:&range])
        {
            if (codepoint >= 0xf700 && codepoint <= 0xf7ff)
            {
                continue;
            }

            preedit->text[preeditTextLength++] = codepoint;
            currentBlockLength++;
        }
    }
    preedit->blockSizes[currentBlockIndex] = currentBlockLength;
    preedit->blockSizesCount = 1 + currentBlockIndex;
    preedit->textCount = preeditTextLength;
    preedit->text[preeditTextLength] = 0;
    preedit->focusedBlockIndex = focusedBlockIndex;
    // The caret is always at the last of preedit in macOS.
    preedit->caretIndex = preeditTextLength;

    _grwlInputPreedit(window);
}

- (void)unmarkText
{
    [[markedText mutableString] setString:@""];
    window->preedit.blockSizesCount = 0;
    window->preedit.textCount = 0;
    window->preedit.focusedBlockIndex = 0;
    window->preedit.caretIndex = 0;
    _grwlInputPreedit(window);
}

- (NSArray*)validAttributesForMarkedText
{
    return [NSArray array];
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    int x = window->preedit.cursorPosX;
    int y = window->preedit.cursorPosY;
    int w = window->preedit.cursorWidth;
    int h = window->preedit.cursorHeight;

    const NSRect frame = [window->ns.object contentRectForFrameRect:[window->ns.object frame]];

    return NSMakeRect(frame.origin.x + x,
                      // The y-axis is upward on macOS, so this conversion is needed.
                      frame.origin.y + frame.size.height - y - h, w, h);
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
    NSString* characters;
    NSEvent* event = [NSApp currentEvent];
    const int mods = translateFlags([event modifierFlags]);
    const int plain = !(mods & GRWL_MOD_SUPER);

    if ([string isKindOfClass:[NSAttributedString class]])
    {
        characters = [string string];
    }
    else
    {
        characters = (NSString*)string;
    }

    NSRange range = NSMakeRange(0, [characters length]);
    while (range.length)
    {
        uint32_t codepoint = 0;

        if ([characters getBytes:&codepoint
                       maxLength:sizeof(codepoint)
                      usedLength:NULL
                        encoding:NSUTF32StringEncoding
                         options:0
                           range:range
                  remainingRange:&range])
        {
            if (codepoint >= 0xf700 && codepoint <= 0xf7ff)
            {
                continue;
            }

            _grwlInputChar(window, codepoint, mods, plain);
        }
    }

    [self unmarkText];
}

- (void)doCommandBySelector:(SEL)selector
{
}

@end

//------------------------------------------------------------------------
// GRWL window class
//------------------------------------------------------------------------

@interface GRWLWindow: NSWindow
{
}
@end

@implementation GRWLWindow

- (BOOL)canBecomeKeyWindow
{
    // Required for NSWindowStyleMaskBorderless windows
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

@end

// Create the Cocoa window
//
static GRWLbool createNativeWindow(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLfbconfig* fbconfig)
{
    window->ns.delegate = [[GRWLWindowDelegate alloc] initWithGlfwWindow:window];
    if (window->ns.delegate == nil)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Failed to create window delegate");
        return GRWL_FALSE;
    }

    NSRect contentRect;

    if (window->monitor)
    {
        GRWLvidmode mode;
        int xpos, ypos;

        _grwlGetVideoModeCocoa(window->monitor, &mode);
        _grwlGetMonitorPosCocoa(window->monitor, &xpos, &ypos);

        contentRect = NSMakeRect(xpos, ypos, mode.width, mode.height);
    }
    else
    {
        if (wndconfig->xpos == GRWL_ANY_POSITION || wndconfig->ypos == GRWL_ANY_POSITION)
        {
            contentRect = NSMakeRect(0, 0, wndconfig->width, wndconfig->height);
        }
        else
        {
            const int xpos = wndconfig->xpos;
            const int ypos = _grwlTransformYCocoa(wndconfig->ypos + wndconfig->height - 1);
            contentRect = NSMakeRect(xpos, ypos, wndconfig->width, wndconfig->height);
        }
    }

    NSUInteger styleMask = NSWindowStyleMaskMiniaturizable;

    if (window->monitor || !window->decorated)
    {
        styleMask |= NSWindowStyleMaskBorderless;
    }
    else
    {
        styleMask |= (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable);

        if (window->resizable)
        {
            styleMask |= NSWindowStyleMaskResizable;
        }
    }

    window->ns.object = [[GRWLWindow alloc] initWithContentRect:contentRect
                                                      styleMask:styleMask
                                                        backing:NSBackingStoreBuffered
                                                          defer:NO];

    if (window->ns.object == nil)
    {
        _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Failed to create window");
        return GRWL_FALSE;
    }

    if (window->monitor)
    {
        if (_grwl.hints.window.softFullscreen)
        {
            [NSApp setPresentationOptions:NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar];
        }
        else
        {
            [window->ns.object setLevel:NSMainMenuWindowLevel + 1];
        }
    }
    else
    {
        if (wndconfig->xpos == GRWL_ANY_POSITION || wndconfig->ypos == GRWL_ANY_POSITION)
        {
            [(NSWindow*)window->ns.object center];
            _grwl.ns.cascadePoint =
                NSPointToCGPoint([window->ns.object cascadeTopLeftFromPoint:NSPointFromCGPoint(_grwl.ns.cascadePoint)]);
        }

        if (wndconfig->resizable)
        {
            const NSWindowCollectionBehavior behavior = NSWindowCollectionBehaviorFullScreenPrimary |
                                                        NSWindowCollectionBehaviorManaged;
            [window->ns.object setCollectionBehavior:behavior];
        }
        else
        {
            const NSWindowCollectionBehavior behavior = NSWindowCollectionBehaviorFullScreenNone;
            [window->ns.object setCollectionBehavior:behavior];
        }

        if (wndconfig->floating)
        {
            [window->ns.object setLevel:NSFloatingWindowLevel];
        }

        if (wndconfig->maximized)
        {
            [window->ns.object zoom:nil];
        }
    }

    if (strlen(wndconfig->ns.frameName))
    {
        [window->ns.object setFrameAutosaveName:@(wndconfig->ns.frameName)];
    }

    window->ns.view = [[GRWLContentView alloc] initWithGlfwWindow:window];
    window->ns.retina = wndconfig->ns.retina;

    if (fbconfig->transparent)
    {
        [window->ns.object setOpaque:NO];
        [window->ns.object setHasShadow:NO];
        [window->ns.object setBackgroundColor:[NSColor clearColor]];
    }

    [window->ns.object setContentView:window->ns.view];
    [window->ns.object makeFirstResponder:window->ns.view];
    [window->ns.object setTitle:@(wndconfig->title)];
    [window->ns.object setDelegate:window->ns.delegate];
    [window->ns.object setAcceptsMouseMovedEvents:YES];
    [window->ns.object setRestorable:NO];

    #if MAC_OS_X_VERSION_MAX_ALLOWED >= 101200
    if ([window->ns.object respondsToSelector:@selector(setTabbingMode:)])
    {
        [window->ns.object setTabbingMode:NSWindowTabbingModeDisallowed];
    }
    #endif

    _grwlGetWindowSizeCocoa(window, &window->ns.width, &window->ns.height);
    _grwlGetFramebufferSizeCocoa(window, &window->ns.fbWidth, &window->ns.fbHeight);

    return GRWL_TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Transforms a y-coordinate between the CG display and NS screen spaces
//
float _grwlTransformYCocoa(float y)
{
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
}

//////////////////////////////////////////////////////////////////////////
//////                       GRWL platform API                      //////
//////////////////////////////////////////////////////////////////////////

GRWLbool _grwlCreateWindowCocoa(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                                const _GRWLfbconfig* fbconfig)
{
    @autoreleasepool
    {

        if (!createNativeWindow(window, wndconfig, fbconfig))
        {
            return GRWL_FALSE;
        }

        if (ctxconfig->client != GRWL_NO_API)
        {
            if (ctxconfig->source == GRWL_NATIVE_CONTEXT_API)
            {
                if (!_grwlInitNSGL())
                {
                    return GRWL_FALSE;
                }
                if (!_grwlCreateContextNSGL(window, ctxconfig, fbconfig))
                {
                    return GRWL_FALSE;
                }
            }
            else if (ctxconfig->source == GRWL_EGL_CONTEXT_API)
            {
                // EGL implementation on macOS use CALayer* EGLNativeWindowType so we
                // need to get the layer for EGL window surface creation.
                [window->ns.view setWantsLayer:YES];
                window->ns.layer = [window->ns.view layer];

                if (!_grwlInitEGL())
                {
                    return GRWL_FALSE;
                }
                if (!_grwlCreateContextEGL(window, ctxconfig, fbconfig))
                {
                    return GRWL_FALSE;
                }
            }
            else if (ctxconfig->source == GRWL_OSMESA_CONTEXT_API)
            {
                if (!_grwlInitOSMesa())
                {
                    return GRWL_FALSE;
                }
                if (!_grwlCreateContextOSMesa(window, ctxconfig, fbconfig))
                {
                    return GRWL_FALSE;
                }
            }

            if (!_grwlRefreshContextAttribs(window, ctxconfig))
            {
                return GRWL_FALSE;
            }
        }

        if (wndconfig->mousePassthrough)
        {
            _grwlSetWindowMousePassthroughCocoa(window, GRWL_TRUE);
        }

        if (window->monitor)
        {
            _grwlShowWindowCocoa(window);
            _grwlFocusWindowCocoa(window);
            acquireMonitor(window);

            if (wndconfig->centerCursor)
            {
                _grwlCenterCursorInContentArea(window);
            }
        }
        else
        {
            if (wndconfig->visible)
            {
                _grwlShowWindowCocoa(window);
                if (wndconfig->focused)
                {
                    _grwlFocusWindowCocoa(window);
                }
            }
        }

        [[NSNotificationCenter defaultCenter] addObserver:window->ns.delegate
                                                 selector:@selector(imeStatusChangeNotified:)
                                                     name:NSTextInputContextKeyboardSelectionDidChangeNotification
                                                   object:nil];

        return GRWL_TRUE;

    } // autoreleasepool
}

void _grwlDestroyWindowCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {

        _grwlSetWindowProgressIndicatorCocoa(window, GRWL_PROGRESS_INDICATOR_DISABLED, 0.0);

        if (_grwl.ns.disabledCursorWindow == window)
        {
            _grwl.ns.disabledCursorWindow = NULL;
        }

        [[NSNotificationCenter defaultCenter] removeObserver:window->ns.delegate];

        [window->ns.object orderOut:nil];

        if (window->monitor)
        {
            releaseMonitor(window);
        }

        if (window->context.destroy)
        {
            window->context.destroy(window);
        }

        [window->ns.object setDelegate:nil];
        [window->ns.delegate release];
        window->ns.delegate = nil;

        [window->ns.view release];
        window->ns.view = nil;

        [window->ns.object close];
        window->ns.object = nil;

        // HACK: Allow Cocoa to catch up before returning
        _grwlPollEventsCocoa();

    } // autoreleasepool
}

void _grwlSetWindowTitleCocoa(_GRWLwindow* window, const char* title)
{
    @autoreleasepool
    {
        NSString* string = @(title);
        [window->ns.object setTitle:string];
        // HACK: Set the miniwindow title explicitly as setTitle: doesn't update it
        //       if the window lacks NSWindowStyleMaskTitled
        [window->ns.object setMiniwindowTitle:string];
    } // autoreleasepool
}

void _grwlSetWindowIconCocoa(_GRWLwindow* window, int count, const GRWLimage* images)
{
    _grwlInputError(GRWL_FEATURE_UNAVAILABLE, "Cocoa: Regular windows do not have icons on macOS");
}

void _grwlSetWindowProgressIndicatorCocoa(_GRWLwindow* window, int progressState, double value)
{
    if (progressState == GRWL_PROGRESS_INDICATOR_ERROR || progressState == GRWL_PROGRESS_INDICATOR_PAUSED)
    {
        progressState = GRWL_PROGRESS_INDICATOR_NORMAL;
    }

    const int oldState = window->ns.dockProgressIndicator.state;
    const int state = progressState;

    const double oldValue = window->ns.dockProgressIndicator.value;

    if (oldState == state)
    {
        if (state == GRWL_PROGRESS_INDICATOR_DISABLED || state == GRWL_PROGRESS_INDICATOR_INDETERMINATE ||
            oldValue == value)
        {
            return;
        }
    }

    if (oldState != state)
    {
        // Reset
        if (oldState == GRWL_PROGRESS_INDICATOR_INDETERMINATE)
        {
            --_grwl.ns.dockProgressIndicator.indeterminateCount;
        }
        if (oldState != GRWL_PROGRESS_INDICATOR_DISABLED)
        {
            --_grwl.ns.dockProgressIndicator.windowCount;
            _grwl.ns.dockProgressIndicator.totalValue -= oldValue;
        }

        // Set
        if (state == GRWL_PROGRESS_INDICATOR_INDETERMINATE)
        {
            ++_grwl.ns.dockProgressIndicator.indeterminateCount;
        }
        if (state != GRWL_PROGRESS_INDICATOR_DISABLED)
        {
            ++_grwl.ns.dockProgressIndicator.windowCount;
            _grwl.ns.dockProgressIndicator.totalValue += value;
        }
    }
    else if (state != GRWL_PROGRESS_INDICATOR_DISABLED)
    {
        _grwl.ns.dockProgressIndicator.totalValue += (value - oldValue);
    }

    if (_grwl.ns.dockProgressIndicator.windowCount > _grwl.ns.dockProgressIndicator.indeterminateCount)
    {
        const double finalValue =
            _grwl.ns.dockProgressIndicator.totalValue / _grwl.ns.dockProgressIndicator.windowCount;
        setDockProgressIndicator(GRWL_PROGRESS_INDICATOR_NORMAL, finalValue);
    }
    else if (_grwl.ns.dockProgressIndicator.indeterminateCount > 0)
    {
        setDockProgressIndicator(GRWL_PROGRESS_INDICATOR_INDETERMINATE, 0.0f);
    }
    else
    {
        setDockProgressIndicator(GRWL_PROGRESS_INDICATOR_DISABLED, 0.0f);
    }

    window->ns.dockProgressIndicator.state = state;
    window->ns.dockProgressIndicator.value = value;
}

void _grwlSetWindowBadgeCocoa(_GRWLwindow* window, int count)
{
    if (window != NULL)
    {
        _grwlInputError(GRWL_FEATURE_UNAVAILABLE,
                        "Cocoa: Cannot set a badge for a window. Pass NULL to set the Dock badge.");
        return;
    }

    if (count == 0)
    {
        [NSApp dockTile].badgeLabel = nil;
        return;
    }

    NSString* string;

    if (count <= 9999)
    {
        string = [@(count) stringValue];
    }
    else
    {
        string = [[@(9999) stringValue] stringByAppendingString:@"+"];
    }

    [NSApp dockTile].badgeLabel = string;
}

void _grwlSetWindowBadgeStringCocoa(_GRWLwindow* window, const char* string)
{
    if (window != NULL)
    {
        _grwlInputError(GRWL_FEATURE_UNAVAILABLE,
                        "Cocoa: Cannot set a badge for a window. Pass NULL to set for the application.");
    }

    if (string == NULL)
    {
        [NSApp dockTile].badgeLabel = nil;
        return;
    }

    NSString* nsString = [NSString stringWithCString:string encoding:[NSString defaultCStringEncoding]];

    [NSApp dockTile].badgeLabel = nsString;
}

void _grwlGetWindowPosCocoa(_GRWLwindow* window, int* xpos, int* ypos)
{
    @autoreleasepool
    {

        const NSRect contentRect = [window->ns.object contentRectForFrameRect:[window->ns.object frame]];

        if (xpos)
        {
            *xpos = contentRect.origin.x;
        }
        if (ypos)
        {
            *ypos = _grwlTransformYCocoa(contentRect.origin.y + contentRect.size.height - 1);
        }

    } // autoreleasepool
}

void _grwlSetWindowPosCocoa(_GRWLwindow* window, int x, int y)
{
    @autoreleasepool
    {

        const NSRect contentRect = [window->ns.view frame];
        const NSRect dummyRect = NSMakeRect(x, _grwlTransformYCocoa(y + contentRect.size.height - 1), 0, 0);
        const NSRect frameRect = [window->ns.object frameRectForContentRect:dummyRect];
        [window->ns.object setFrameOrigin:frameRect.origin];

    } // autoreleasepool
}

void _grwlGetWindowSizeCocoa(_GRWLwindow* window, int* width, int* height)
{
    @autoreleasepool
    {

        const NSRect contentRect = [window->ns.view frame];

        if (width)
        {
            *width = contentRect.size.width;
        }
        if (height)
        {
            *height = contentRect.size.height;
        }

    } // autoreleasepool
}

void _grwlSetWindowSizeCocoa(_GRWLwindow* window, int width, int height)
{
    @autoreleasepool
    {

        if (window->monitor)
        {
            if (window->monitor->window == window)
            {
                acquireMonitor(window);
            }
        }
        else
        {
            NSRect contentRect = [window->ns.object contentRectForFrameRect:[window->ns.object frame]];
            contentRect.origin.y += contentRect.size.height - height;
            contentRect.size = NSMakeSize(width, height);
            [window->ns.object setFrame:[window->ns.object frameRectForContentRect:contentRect] display:YES];
        }

    } // autoreleasepool
}

void _grwlSetWindowSizeLimitsCocoa(_GRWLwindow* window, int minwidth, int minheight, int maxwidth, int maxheight)
{
    @autoreleasepool
    {

        if (minwidth == GRWL_DONT_CARE || minheight == GRWL_DONT_CARE)
        {
            [window->ns.object setContentMinSize:NSMakeSize(0, 0)];
        }
        else
        {
            [window->ns.object setContentMinSize:NSMakeSize(minwidth, minheight)];
        }

        if (maxwidth == GRWL_DONT_CARE || maxheight == GRWL_DONT_CARE)
        {
            [window->ns.object setContentMaxSize:NSMakeSize(DBL_MAX, DBL_MAX)];
        }
        else
        {
            [window->ns.object setContentMaxSize:NSMakeSize(maxwidth, maxheight)];
        }

    } // autoreleasepool
}

void _grwlSetWindowAspectRatioCocoa(_GRWLwindow* window, int numer, int denom)
{
    @autoreleasepool
    {
        if (numer == GRWL_DONT_CARE || denom == GRWL_DONT_CARE)
        {
            [window->ns.object setResizeIncrements:NSMakeSize(1.0, 1.0)];
        }
        else
        {
            [window->ns.object setContentAspectRatio:NSMakeSize(numer, denom)];
        }
    } // autoreleasepool
}

void _grwlGetFramebufferSizeCocoa(_GRWLwindow* window, int* width, int* height)
{
    @autoreleasepool
    {

        const NSRect contentRect = [window->ns.view frame];
        const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];

        if (width)
        {
            *width = (int)fbRect.size.width;
        }
        if (height)
        {
            *height = (int)fbRect.size.height;
        }

    } // autoreleasepool
}

void _grwlGetWindowFrameSizeCocoa(_GRWLwindow* window, int* left, int* top, int* right, int* bottom)
{
    @autoreleasepool
    {

        const NSRect contentRect = [window->ns.view frame];
        const NSRect frameRect = [window->ns.object frameRectForContentRect:contentRect];

        if (left)
        {
            *left = contentRect.origin.x - frameRect.origin.x;
        }
        if (top)
        {
            *top = frameRect.origin.y + frameRect.size.height - contentRect.origin.y - contentRect.size.height;
        }
        if (right)
        {
            *right = frameRect.origin.x + frameRect.size.width - contentRect.origin.x - contentRect.size.width;
        }
        if (bottom)
        {
            *bottom = contentRect.origin.y - frameRect.origin.y;
        }

    } // autoreleasepool
}

void _grwlGetWindowContentScaleCocoa(_GRWLwindow* window, float* xscale, float* yscale)
{
    @autoreleasepool
    {

        const NSRect points = [window->ns.view frame];
        const NSRect pixels = [window->ns.view convertRectToBacking:points];

        if (xscale)
        {
            *xscale = (float)(pixels.size.width / points.size.width);
        }
        if (yscale)
        {
            *yscale = (float)(pixels.size.height / points.size.height);
        }

    } // autoreleasepool
}

void _grwlIconifyWindowCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        [window->ns.object miniaturize:nil];
    } // autoreleasepool
}

void _grwlRestoreWindowCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        if ([window->ns.object isMiniaturized])
        {
            [window->ns.object deminiaturize:nil];
        }
        else if ([window->ns.object isZoomed])
        {
            [window->ns.object zoom:nil];
        }
    } // autoreleasepool
}

void _grwlMaximizeWindowCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        if (![window->ns.object isZoomed])
        {
            [window->ns.object zoom:nil];
        }
    } // autoreleasepool
}

void _grwlShowWindowCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        [window->ns.object orderFront:nil];
    } // autoreleasepool
}

void _grwlHideWindowCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        [window->ns.object orderOut:nil];
    } // autoreleasepool
}

void _grwlRequestWindowAttentionCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        [NSApp requestUserAttention:NSInformationalRequest];
    } // autoreleasepool
}

void _grwlFocusWindowCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        // Make us the active application
        // HACK: This is here to prevent applications using only hidden windows from
        //       being activated, but should probably not be done every time any
        //       window is shown
        [NSApp activateIgnoringOtherApps:YES];
        [window->ns.object makeKeyAndOrderFront:nil];
    } // autoreleasepool
}

void _grwlSetWindowMonitorCocoa(_GRWLwindow* window, _GRWLmonitor* monitor, int xpos, int ypos, int width, int height,
                                int refreshRate)
{
    @autoreleasepool
    {

        if (window->monitor == monitor)
        {
            if (monitor)
            {
                if (monitor->window == window)
                {
                    acquireMonitor(window);
                }
            }
            else
            {
                const NSRect contentRect = NSMakeRect(xpos, _grwlTransformYCocoa(ypos + height - 1), width, height);
                const NSUInteger styleMask = [window->ns.object styleMask];
                const NSRect frameRect = [window->ns.object frameRectForContentRect:contentRect styleMask:styleMask];

                [window->ns.object setFrame:frameRect display:YES];
            }

            return;
        }

        if (window->monitor)
        {
            releaseMonitor(window);
        }

        _grwlInputWindowMonitor(window, monitor);

        // HACK: Allow the state cached in Cocoa to catch up to reality
        // TODO: Solve this in a less terrible way
        _grwlPollEventsCocoa();

        NSUInteger styleMask = [window->ns.object styleMask];

        if (window->monitor)
        {
            styleMask &= ~(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable);
            styleMask |= NSWindowStyleMaskBorderless;
        }
        else
        {
            if (window->decorated)
            {
                styleMask &= ~NSWindowStyleMaskBorderless;
                styleMask |= (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable);
            }

            if (window->resizable)
            {
                styleMask |= NSWindowStyleMaskResizable;
            }
            else
            {
                styleMask &= ~NSWindowStyleMaskResizable;
            }
        }

        [window->ns.object setStyleMask:styleMask];
        // HACK: Changing the style mask can cause the first responder to be cleared
        [window->ns.object makeFirstResponder:window->ns.view];

        if (window->monitor)
        {
            if (_grwl.hints.window.softFullscreen)
            {
                [NSApp setPresentationOptions:NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar];
            }
            else
            {
                [window->ns.object setLevel:NSMainMenuWindowLevel + 1];
            }

            [window->ns.object setHasShadow:NO];

            acquireMonitor(window);
        }
        else
        {
            NSRect contentRect = NSMakeRect(xpos, _grwlTransformYCocoa(ypos + height - 1), width, height);
            NSRect frameRect = [window->ns.object frameRectForContentRect:contentRect styleMask:styleMask];
            [window->ns.object setFrame:frameRect display:YES];

            if (window->numer != GRWL_DONT_CARE && window->denom != GRWL_DONT_CARE)
            {
                [window->ns.object setContentAspectRatio:NSMakeSize(window->numer, window->denom)];
            }

            if (window->minwidth != GRWL_DONT_CARE && window->minheight != GRWL_DONT_CARE)
            {
                [window->ns.object setContentMinSize:NSMakeSize(window->minwidth, window->minheight)];
            }

            if (window->maxwidth != GRWL_DONT_CARE && window->maxheight != GRWL_DONT_CARE)
            {
                [window->ns.object setContentMaxSize:NSMakeSize(window->maxwidth, window->maxheight)];
            }

            if (window->floating)
            {
                [window->ns.object setLevel:NSFloatingWindowLevel];
            }
            else
            {
                [window->ns.object setLevel:NSNormalWindowLevel];
            }

            if (window->resizable)
            {
                const NSWindowCollectionBehavior behavior = NSWindowCollectionBehaviorFullScreenPrimary |
                                                            NSWindowCollectionBehaviorManaged;
                [window->ns.object setCollectionBehavior:behavior];
            }
            else
            {
                const NSWindowCollectionBehavior behavior = NSWindowCollectionBehaviorFullScreenNone;
                [window->ns.object setCollectionBehavior:behavior];
            }

            if (_grwl.hints.window.softFullscreen)
            {
                [NSApp setPresentationOptions:NSApplicationPresentationDefault];
            }

            [window->ns.object setHasShadow:YES];
            // HACK: Clearing NSWindowStyleMaskTitled resets and disables the window
            //       title property but the miniwindow title property is unaffected
            [window->ns.object setTitle:[window->ns.object miniwindowTitle]];
        }

    } // autoreleasepool
}

GRWLbool _grwlWindowFocusedCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        return [window->ns.object isKeyWindow];
    } // autoreleasepool
}

GRWLbool _grwlWindowIconifiedCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        return [window->ns.object isMiniaturized];
    } // autoreleasepool
}

GRWLbool _grwlWindowVisibleCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        return [window->ns.object isVisible];
    } // autoreleasepool
}

GRWLbool _grwlWindowMaximizedCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {

        if (window->resizable)
        {
            return [window->ns.object isZoomed];
        }
        else
        {
            return GRWL_FALSE;
        }

    } // autoreleasepool
}

GRWLbool _grwlWindowHoveredCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {

        const NSPoint point = [NSEvent mouseLocation];

        if ([NSWindow windowNumberAtPoint:point belowWindowWithWindowNumber:0] != [window->ns.object windowNumber])
        {
            return GRWL_FALSE;
        }

        return NSMouseInRect(point, [window->ns.object convertRectToScreen:[window->ns.view frame]], NO);

    } // autoreleasepool
}

GRWLbool _grwlFramebufferTransparentCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        return ![window->ns.object isOpaque] && ![window->ns.view isOpaque];
    } // autoreleasepool
}

void _grwlSetWindowResizableCocoa(_GRWLwindow* window, GRWLbool enabled)
{
    @autoreleasepool
    {

        const NSUInteger styleMask = [window->ns.object styleMask];
        if (enabled)
        {
            [window->ns.object setStyleMask:(styleMask | NSWindowStyleMaskResizable)];
            const NSWindowCollectionBehavior behavior = NSWindowCollectionBehaviorFullScreenPrimary |
                                                        NSWindowCollectionBehaviorManaged;
            [window->ns.object setCollectionBehavior:behavior];
        }
        else
        {
            [window->ns.object setStyleMask:(styleMask & ~NSWindowStyleMaskResizable)];
            const NSWindowCollectionBehavior behavior = NSWindowCollectionBehaviorFullScreenNone;
            [window->ns.object setCollectionBehavior:behavior];
        }

    } // autoreleasepool
}

void _grwlSetWindowDecoratedCocoa(_GRWLwindow* window, GRWLbool enabled)
{
    @autoreleasepool
    {

        NSUInteger styleMask = [window->ns.object styleMask];
        if (enabled)
        {
            styleMask |= (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable);
            styleMask &= ~NSWindowStyleMaskBorderless;
        }
        else
        {
            styleMask |= NSWindowStyleMaskBorderless;
            styleMask &= ~(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable);
        }

        [window->ns.object setStyleMask:styleMask];
        [window->ns.object makeFirstResponder:window->ns.view];

    } // autoreleasepool
}

void _grwlSetWindowFloatingCocoa(_GRWLwindow* window, GRWLbool enabled)
{
    @autoreleasepool
    {
        if (enabled)
        {
            [window->ns.object setLevel:NSFloatingWindowLevel];
        }
        else
        {
            [window->ns.object setLevel:NSNormalWindowLevel];
        }
    } // autoreleasepool
}

void _grwlSetWindowMousePassthroughCocoa(_GRWLwindow* window, GRWLbool enabled)
{
    @autoreleasepool
    {
        [window->ns.object setIgnoresMouseEvents:enabled];
    }
}

float _grwlGetWindowOpacityCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {
        return (float)[window->ns.object alphaValue];
    } // autoreleasepool
}

void _grwlSetWindowOpacityCocoa(_GRWLwindow* window, float opacity)
{
    @autoreleasepool
    {
        [window->ns.object setAlphaValue:opacity];
    } // autoreleasepool
}

void _grwlSetRawMouseMotionCocoa(_GRWLwindow* window, GRWLbool enabled)
{
    _grwlInputError(GRWL_FEATURE_UNIMPLEMENTED, "Cocoa: Raw mouse motion not yet implemented");
}

GRWLbool _grwlRawMouseMotionSupportedCocoa(void)
{
    return GRWL_FALSE;
}

void _grwlPollEventsCocoa(void)
{
    @autoreleasepool
    {

        for (;;)
        {
            NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                untilDate:[NSDate distantPast]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
            if (event == nil)
            {
                break;
            }

            [NSApp sendEvent:event];
        }

    } // autoreleasepool
}

void _grwlWaitEventsCocoa(void)
{
    @autoreleasepool
    {

        // I wanted to pass NO to dequeue:, and rely on PollEvents to
        // dequeue and send.  For reasons not at all clear to me, passing
        // NO to dequeue: causes this method never to return.
        NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:[NSDate distantFuture]
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        [NSApp sendEvent:event];

        _grwlPollEventsCocoa();

    } // autoreleasepool
}

void _grwlWaitEventsTimeoutCocoa(double timeout)
{
    @autoreleasepool
    {

        NSDate* date = [NSDate dateWithTimeIntervalSinceNow:timeout];
        NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:date
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        if (event)
        {
            [NSApp sendEvent:event];
        }

        _grwlPollEventsCocoa();

    } // autoreleasepool
}

void _grwlPostEmptyEventCocoa(void)
{
    @autoreleasepool
    {

        NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                            location:NSMakePoint(0, 0)
                                       modifierFlags:0
                                           timestamp:0
                                        windowNumber:0
                                             context:nil
                                             subtype:0
                                               data1:0
                                               data2:0];
        [NSApp postEvent:event atStart:YES];

    } // autoreleasepool
}

void _grwlGetCursorPosCocoa(_GRWLwindow* window, double* xpos, double* ypos)
{
    @autoreleasepool
    {

        const NSRect contentRect = [window->ns.view frame];
        // NOTE: The returned location uses base 0,1 not 0,0
        const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];

        if (xpos)
        {
            *xpos = pos.x;
        }
        if (ypos)
        {
            *ypos = contentRect.size.height - pos.y;
        }

    } // autoreleasepool
}

void _grwlSetCursorPosCocoa(_GRWLwindow* window, double x, double y)
{
    @autoreleasepool
    {

        updateCursorImage(window);

        const NSRect contentRect = [window->ns.view frame];
        // NOTE: The returned location uses base 0,1 not 0,0
        const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];

        window->ns.cursorWarpDeltaX += x - pos.x;
        window->ns.cursorWarpDeltaY += y - contentRect.size.height + pos.y;

        if (window->monitor)
        {
            CGDisplayMoveCursorToPoint(window->monitor->ns.displayID, CGPointMake(x, y));
        }
        else
        {
            const NSRect localRect = NSMakeRect(x, contentRect.size.height - y - 1, 0, 0);
            const NSRect globalRect = [window->ns.object convertRectToScreen:localRect];
            const NSPoint globalPoint = globalRect.origin;

            CGWarpMouseCursorPosition(CGPointMake(globalPoint.x, _grwlTransformYCocoa(globalPoint.y)));
        }

        // HACK: Calling this right after setting the cursor position prevents macOS
        //       from freezing the cursor for a fraction of a second afterwards
        if (window->cursorMode != GRWL_CURSOR_DISABLED)
        {
            CGAssociateMouseAndMouseCursorPosition(true);
        }

    } // autoreleasepool
}

void _grwlSetCursorModeCocoa(_GRWLwindow* window, int mode)
{
    @autoreleasepool
    {

        if (mode == GRWL_CURSOR_CAPTURED)
        {
            _grwlInputError(GRWL_FEATURE_UNIMPLEMENTED, "Cocoa: Captured cursor mode not yet implemented");
        }

        if (_grwlWindowFocusedCocoa(window))
        {
            updateCursorMode(window);
        }

    } // autoreleasepool
}

const char* _grwlGetScancodeNameCocoa(int scancode)
{
    @autoreleasepool
    {

        if (scancode < 0 || scancode > 0xff || _grwl.ns.keycodes[scancode] == GRWL_KEY_UNKNOWN)
        {
            _grwlInputError(GRWL_INVALID_VALUE, "Invalid scancode %i", scancode);
            return NULL;
        }

        if (!_grwl.ns.unicodeData)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Keyboard Unicode data missing");
            return NULL;
        }

        const int key = _grwl.ns.keycodes[scancode];

        UInt32 deadKeyState = 0;
        UniChar characters[4];
        UniCharCount characterCount = 0;

        if (UCKeyTranslate([(NSData*)_grwl.ns.unicodeData bytes], scancode, kUCKeyActionDisplay, 0, LMGetKbdType(),
                           kUCKeyTranslateNoDeadKeysBit, &deadKeyState, sizeof(characters) / sizeof(characters[0]),
                           &characterCount, characters) != noErr)
        {
            return NULL;
        }

        if (!characterCount)
        {
            return NULL;
        }

        CFStringRef string =
            CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, characters, characterCount, kCFAllocatorNull);
        CFStringGetCString(string, _grwl.ns.keynames[key], sizeof(_grwl.ns.keynames[key]), kCFStringEncodingUTF8);
        CFRelease(string);

        return _grwl.ns.keynames[key];

    } // autoreleasepool
}

int _grwlGetKeyScancodeCocoa(int key)
{
    return _grwl.ns.scancodes[key];
}

const char* _grwlGetKeyboardLayoutNameCocoa(void)
{
    TISInputSourceRef source = TISCopyCurrentKeyboardInputSource();
    NSString* name = (__bridge NSString*)TISGetInputSourceProperty(source, kTISPropertyLocalizedName);
    if (!name)
    {
        CFRelease(source);
        _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Failed to retrieve keyboard layout name");
        return NULL;
    }

    free(_grwl.ns.keyboardLayoutName);
    _grwl.ns.keyboardLayoutName = _grwl_strdup([name UTF8String]);

    CFRelease(source);
    return _grwl.ns.keyboardLayoutName;
}

GRWLbool _grwlCreateCursorCocoa(_GRWLcursor* cursor, const GRWLimage* image, int xhot, int yhot)
{
    @autoreleasepool
    {

        NSImage* native;
        NSBitmapImageRep* rep;

        rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                      pixelsWide:image->width
                                                      pixelsHigh:image->height
                                                   bitsPerSample:8
                                                 samplesPerPixel:4
                                                        hasAlpha:YES
                                                        isPlanar:NO
                                                  colorSpaceName:NSCalibratedRGBColorSpace
                                                    bitmapFormat:NSBitmapFormatAlphaNonpremultiplied
                                                     bytesPerRow:image->width * 4
                                                    bitsPerPixel:32];

        if (rep == nil)
        {
            return GRWL_FALSE;
        }

        memcpy([rep bitmapData], image -> pixels, image -> width * image -> height * 4);

        native = [[NSImage alloc] initWithSize:NSMakeSize(image->width, image->height)];
        [native addRepresentation:rep];

        cursor->ns.object = [[NSCursor alloc] initWithImage:native hotSpot:NSMakePoint(xhot, yhot)];

        [native release];
        [rep release];

        if (cursor->ns.object == nil)
        {
            return GRWL_FALSE;
        }

        return GRWL_TRUE;

    } // autoreleasepool
}

GRWLbool _grwlCreateStandardCursorCocoa(_GRWLcursor* cursor, int shape)
{
    @autoreleasepool
    {

        SEL cursorSelector = NULL;

        // HACK: Try to use a private message
        switch (shape)
        {
            case GRWL_RESIZE_EW_CURSOR:
                cursorSelector = NSSelectorFromString(@"_windowResizeEastWestCursor");
                break;
            case GRWL_RESIZE_NS_CURSOR:
                cursorSelector = NSSelectorFromString(@"_windowResizeNorthSouthCursor");
                break;
            case GRWL_RESIZE_NWSE_CURSOR:
                cursorSelector = NSSelectorFromString(@"_windowResizeNorthWestSouthEastCursor");
                break;
            case GRWL_RESIZE_NESW_CURSOR:
                cursorSelector = NSSelectorFromString(@"_windowResizeNorthEastSouthWestCursor");
                break;
        }

        if (cursorSelector && [NSCursor respondsToSelector:cursorSelector])
        {
            id object = [NSCursor performSelector:cursorSelector];
            if ([object isKindOfClass:[NSCursor class]])
            {
                cursor->ns.object = object;
            }
        }

        if (!cursor->ns.object)
        {
            switch (shape)
            {
                case GRWL_ARROW_CURSOR:
                    cursor->ns.object = [NSCursor arrowCursor];
                    break;
                case GRWL_IBEAM_CURSOR:
                    cursor->ns.object = [NSCursor IBeamCursor];
                    break;
                case GRWL_CROSSHAIR_CURSOR:
                    cursor->ns.object = [NSCursor crosshairCursor];
                    break;
                case GRWL_POINTING_HAND_CURSOR:
                    cursor->ns.object = [NSCursor pointingHandCursor];
                    break;
                case GRWL_RESIZE_EW_CURSOR:
                    cursor->ns.object = [NSCursor resizeLeftRightCursor];
                    break;
                case GRWL_RESIZE_NS_CURSOR:
                    cursor->ns.object = [NSCursor resizeUpDownCursor];
                    break;
                case GRWL_RESIZE_ALL_CURSOR:
                    cursor->ns.object = [NSCursor closedHandCursor];
                    break;
                case GRWL_NOT_ALLOWED_CURSOR:
                    cursor->ns.object = [NSCursor operationNotAllowedCursor];
                    break;
            }
        }

        if (!cursor->ns.object)
        {
            _grwlInputError(GRWL_CURSOR_UNAVAILABLE, "Cocoa: Standard cursor shape unavailable");
            return GRWL_FALSE;
        }

        [cursor->ns.object retain];
        return GRWL_TRUE;

    } // autoreleasepool
}

void _grwlDestroyCursorCocoa(_GRWLcursor* cursor)
{
    @autoreleasepool
    {
        if (cursor->ns.object)
        {
            [(NSCursor*)cursor->ns.object release];
        }
    } // autoreleasepool
}

void _grwlSetCursorCocoa(_GRWLwindow* window, _GRWLcursor* cursor)
{
    @autoreleasepool
    {
        if (cursorInContentArea(window))
        {
            updateCursorImage(window);
        }
    } // autoreleasepool
}

void _grwlSetClipboardStringCocoa(const char* string)
{
    @autoreleasepool
    {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard declareTypes:@[ NSPasteboardTypeString ] owner:nil];
        [pasteboard setString:@(string) forType:NSPasteboardTypeString];
    } // autoreleasepool
}

const char* _grwlGetClipboardStringCocoa(void)
{
    @autoreleasepool
    {

        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];

        if (![[pasteboard types] containsObject:NSPasteboardTypeString])
        {
            _grwlInputError(GRWL_FORMAT_UNAVAILABLE, "Cocoa: Failed to retrieve string from pasteboard");
            return NULL;
        }

        NSString* object = [pasteboard stringForType:NSPasteboardTypeString];
        if (!object)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Failed to retrieve object from pasteboard");
            return NULL;
        }

        _grwl_free(_grwl.ns.clipboardString);
        _grwl.ns.clipboardString = _grwl_strdup([object UTF8String]);

        return _grwl.ns.clipboardString;

    } // autoreleasepool
}

void _grwlUpdatePreeditCursorRectangleCocoa(_GRWLwindow* window)
{
    // Do nothing. Instead, implement `firstRectForCharacterRange` callback
    // to update the position.
}

void _grwlResetPreeditTextCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {

        NSTextInputContext* context = [NSTextInputContext currentInputContext];
        [context discardMarkedText];
        [window->ns.view unmarkText];

    } // autoreleasepool
}

void _grwlSetIMEStatusCocoa(_GRWLwindow* window, int active)
{
    @autoreleasepool
    {

        if (active)
        {
            NSArray* locales = CFBridgingRelease(CFLocaleCopyPreferredLanguages());
            // Select the most preferred locale.
            CFStringRef locale = (__bridge CFStringRef)[locales firstObject];
            if (locale)
            {
                TISInputSourceRef source = TISCopyInputSourceForLanguage(locale);
                if (source)
                {
                    CFStringRef sourceType = TISGetInputSourceProperty(source, kTISPropertyInputSourceType);

                    if (sourceType != kTISTypeKeyboardInputMethodModeEnabled)
                    {
                        TISSelectInputSource(source);
                    }
                    else
                    {
                        // Some IMEs return a input-method that has input-method-modes for
                        // `TISCopyInputSourceForLanguage()`. We can't select these input-methods directly, but need to
                        // find a input-method-mode of the input-method. Example:
                        //  - Input Method: com.apple.inputmethod.SCIM
                        //  - Input Mode: com.apple.inputmethod.SCIM.ITABC
                        NSString* sourceID =
                            (__bridge NSString*)TISGetInputSourceProperty(source, kTISPropertyInputSourceID);
                        NSDictionary* properties = @{
                            (__bridge NSString*)
                            kTISPropertyInputSourceCategory : (__bridge NSString*)kTISCategoryKeyboardInputSource,
                            (__bridge NSString*)kTISPropertyInputSourceIsSelectCapable : @YES,
                        };
                        NSArray* selectableSources =
                            CFBridgingRelease(TISCreateInputSourceList((__bridge CFDictionaryRef)properties, NO));
                        for (id sourceCandidate in selectableSources)
                        {
                            TISInputSourceRef sourceCandidateRef = (__bridge TISInputSourceRef)sourceCandidate;
                            NSString* sourceCandidateID = (__bridge NSString*)TISGetInputSourceProperty(
                                sourceCandidateRef, kTISPropertyInputSourceID);
                            if ([sourceCandidateID hasPrefix:sourceID])
                            {
                                TISSelectInputSource(sourceCandidateRef);
                                break;
                            }
                        }
                    }

                    CFRelease(source);
                }
            }
        }
        else
        {
            TISInputSourceRef source = TISCopyCurrentASCIICapableKeyboardInputSource();
            TISSelectInputSource(source);
            CFRelease(source);
        }

        // `NSTextInputContextKeyboardSelectionDidChangeNotification` is sometimes
        // not called immediately after this, so call the callback here.
        _grwlInputIMEStatus(window);

    } // autoreleasepool
}

int _grwlGetIMEStatusCocoa(_GRWLwindow* window)
{
    @autoreleasepool
    {

        NSArray* asciiInputSources = CFBridgingRelease(TISCreateASCIICapableInputSourceList());

        TISInputSourceRef currentSource = TISCopyCurrentKeyboardInputSource();
        NSString* currentSourceID =
            (__bridge NSString*)TISGetInputSourceProperty(currentSource, kTISPropertyInputSourceID);
        CFRelease(currentSource);

        for (int i = 0; i < [asciiInputSources count]; i++)
        {
            TISInputSourceRef asciiSource = (__bridge TISInputSourceRef)[asciiInputSources objectAtIndex:i];
            NSString* asciiSourceID =
                (__bridge NSString*)TISGetInputSourceProperty(asciiSource, kTISPropertyInputSourceID);
            if ([asciiSourceID compare:currentSourceID] == NSOrderedSame)
            {
                return GRWL_FALSE;
            }
        }

        return GRWL_TRUE;

    } // autoreleasepool
}

EGLenum _grwlGetEGLPlatformCocoa(EGLint** attribs)
{
    if (_grwl.egl.ANGLE_platform_angle)
    {
        int type = 0;

        if (_grwl.egl.ANGLE_platform_angle_opengl)
        {
            if (_grwl.hints.init.angleType == GRWL_ANGLE_PLATFORM_TYPE_OPENGL)
            {
                type = EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE;
            }
        }

        if (_grwl.egl.ANGLE_platform_angle_metal)
        {
            if (_grwl.hints.init.angleType == GRWL_ANGLE_PLATFORM_TYPE_METAL)
            {
                type = EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE;
            }
        }

        if (type)
        {
            *attribs = _grwl_calloc(3, sizeof(EGLint));
            (*attribs)[0] = EGL_PLATFORM_ANGLE_TYPE_ANGLE;
            (*attribs)[1] = type;
            (*attribs)[2] = EGL_NONE;
            return EGL_PLATFORM_ANGLE_ANGLE;
        }
    }

    return 0;
}

EGLNativeDisplayType _grwlGetEGLNativeDisplayCocoa(void)
{
    return EGL_DEFAULT_DISPLAY;
}

EGLNativeWindowType _grwlGetEGLNativeWindowCocoa(_GRWLwindow* window)
{
    return window->ns.layer;
}

void _grwlGetRequiredInstanceExtensionsCocoa(char** extensions)
{
    if (_grwl.vk.KHR_surface && _grwl.vk.EXT_metal_surface)
    {
        extensions[0] = "VK_KHR_surface";
        extensions[1] = "VK_EXT_metal_surface";
    }
    else if (_grwl.vk.KHR_surface && _grwl.vk.MVK_macos_surface)
    {
        extensions[0] = "VK_KHR_surface";
        extensions[1] = "VK_MVK_macos_surface";
    }
}

GRWLbool _grwlGetPhysicalDevicePresentationSupportCocoa(VkInstance instance, VkPhysicalDevice device,
                                                        uint32_t queuefamily)
{
    return GRWL_TRUE;
}

VkResult _grwlCreateWindowSurfaceCocoa(VkInstance instance, _GRWLwindow* window, const VkAllocationCallbacks* allocator,
                                       VkSurfaceKHR* surface)
{
    @autoreleasepool
    {

    #if MAC_OS_X_VERSION_MAX_ALLOWED >= 101100
        // HACK: Dynamically load Core Animation to avoid adding an extra
        //       dependency for the majority who don't use MoltenVK
        NSBundle* bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/QuartzCore.framework"];
        if (!bundle)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Failed to find QuartzCore.framework");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        // NOTE: Create the layer here as makeBackingLayer should not return nil
        window->ns.layer = [[bundle classNamed:@"CAMetalLayer"] layer];
        if (!window->ns.layer)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Failed to create layer for view");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        if (window->ns.retina)
        {
            [window->ns.layer setContentsScale:[window->ns.object backingScaleFactor]];
        }

        [window->ns.view setLayer:window->ns.layer];
        [window->ns.view setWantsLayer:YES];

        VkResult err;

        if (_grwl.vk.EXT_metal_surface)
        {
            VkMetalSurfaceCreateInfoEXT sci;

            PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT;
            vkCreateMetalSurfaceEXT =
                (PFN_vkCreateMetalSurfaceEXT)vkGetInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT");
            if (!vkCreateMetalSurfaceEXT)
            {
                _grwlInputError(GRWL_API_UNAVAILABLE, "Cocoa: Vulkan instance missing VK_EXT_metal_surface extension");
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }

            memset(&sci, 0, sizeof(sci));
            sci.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
            sci.pLayer = window->ns.layer;

            err = vkCreateMetalSurfaceEXT(instance, &sci, allocator, surface);
        }
        else
        {
            VkMacOSSurfaceCreateInfoMVK sci;

            PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK;
            vkCreateMacOSSurfaceMVK =
                (PFN_vkCreateMacOSSurfaceMVK)vkGetInstanceProcAddr(instance, "vkCreateMacOSSurfaceMVK");
            if (!vkCreateMacOSSurfaceMVK)
            {
                _grwlInputError(GRWL_API_UNAVAILABLE, "Cocoa: Vulkan instance missing VK_MVK_macos_surface extension");
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }

            memset(&sci, 0, sizeof(sci));
            sci.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
            sci.pView = window->ns.view;

            err = vkCreateMacOSSurfaceMVK(instance, &sci, allocator, surface);
        }

        if (err)
        {
            _grwlInputError(GRWL_PLATFORM_ERROR, "Cocoa: Failed to create Vulkan surface: %s",
                            _grwlGetVulkanResultString(err));
        }

        return err;
    #else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    #endif

    } // autoreleasepool
}

_GRWLusercontext* _grwlCreateUserContextCocoa(_GRWLwindow* window)
{
    if (window->context.nsgl.object)
    {
        return _grwlCreateUserContextNSGL(window);
    }
    else if (window->context.egl.handle)
    {
        return _grwlCreateUserContextEGL(window);
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////////
//////                        GRWL native API                       //////
//////////////////////////////////////////////////////////////////////////

GRWLAPI id grwlGetCocoaWindow(GRWLwindow* handle)
{
    _GRWLwindow* window = (_GRWLwindow*)handle;
    _GRWL_REQUIRE_INIT_OR_RETURN(nil);

    if (_grwl.platform.platformID != GRWL_PLATFORM_COCOA)
    {
        _grwlInputError(GRWL_PLATFORM_UNAVAILABLE, "Cocoa: Platform not initialized");
        return NULL;
    }

    return window->ns.object;
}

#endif // _GRWL_COCOA
