#include <Geode/Geode.hpp>
#include <imgui.h>
#include <imgui-cocos.hpp>

#define CommentType CommentTypeDummy
#import "Cocoa/Cocoa.h"
#undef CommentType

float ImGuiCocos::retinaFactor() {
    float displayScale = 1.f;
    if ([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)]) {
        NSArray* screens = [NSScreen screens];
        for (int i = 0; i < screens.count; i++) {
            float s = [screens[i] backingScaleFactor];
            if (s > displayScale)
                displayScale = s;
        }
    }
    return displayScale;
}
