#include <Geode/Geode.hpp>
#include <imgui.h>
#include <imgui-cocos.hpp>

#define CommentType CommentTypeDummy
#import "Cocoa/Cocoa.h"
#undef CommentType

float ImGuiCocos::retinaCheck() {
    float displayScale = 1.f;
    if ([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)]) {
        NSArray *screens = [NSScreen screens];
        NSUInteger screenCount = screens.count;
        for (int i = 0; i < screenCount; i++) {
            float s = [screens[i] backingScaleFactor];
            if (s > displayScale)
                displayScale = s;
        }
    }
    return displayScale;
}