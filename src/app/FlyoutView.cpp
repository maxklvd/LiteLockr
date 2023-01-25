/*
 * Copyright (C) 2020, 2022 Max Kolesnikov <maxklvd@gmail.com>
 *
 * This file is part of LiteLockr.
 *
 * LiteLockr is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LiteLockr is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LiteLockr.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FlyoutView.h"

#include <numbers>

#include "gfx/BitmapUtils.h"
#include "ini/SettingsData.h"
#include "lock/HookThread.h"
#include "sys/Comparison.h"
#include "sys/KeyFrames.h"

#include "res/Resources.h"

namespace litelockr {

void FlyoutView::initialize(HWND /*hWnd*/) {
    assert(!hasInitialized_);
    guard_.initialize();

    auto& st = SettingsData::instance();

    model_.keyboard.checked = (st.lockKeyboard.value() == 0);
    model_.mouse.checked = (st.lockMouse.value() == 0);

    bool enabled = !(model_.keyboard.checked && model_.mouse.checked);
    model_.setLockState(enabled ? LockState::READY : LockState::DISABLED);

    SIZE size = model_.size();
    buffer_.create(size.cx, size.cy);
    buffer_.clear({0, 0, 0, 0});

    initializeImages();
    initializeScene();
    initializeAnimations();

    checkImageSizes();

    HookThread::setLockStateListener([this](bool isLocked) {
        model_.setLocked(isLocked);
    });

    hasInitialized_ = true;
}

void FlyoutView::initializeImages() {
    //
    // Title
    //
    auto& title = resources_.title;
    title.close.loadPng(IDB_CLOSE);
    title.menu.loadPng(IDB_MENU);
    title.background.loadPng(IDB_TITLE);

    //
    // Option bar
    //
    auto& opt = resources_.menuBar;
    opt.settings.loadPng(IDB_SETTINGS);
    opt.help.loadPng(IDB_HELP);
    opt.about.loadPng(IDB_ABOUT);

    //
    // Body
    //
    auto& body = resources_.body;
    body.progressFull.loadPng(IDB_PROGRESS_FULL);
}

void FlyoutView::drawMenuBarMaterials() {
    const auto& m = model_;
    auto& opt = resources_.menuBar;
    auto& scene = resources_.scene;

    Bitmap background;
    background.create(m.menuBar.w, m.menuBar.h);
    background.clear(m.menuBarBackground);

    auto& left = scene.planeLeftMat.bitmap;
    auto& right = scene.planeRightMat.bitmap;
    left.create(m.menuBar.w / 2, m.menuBar.h);
    right.create(m.menuBar.w - m.menuBar.w / 2, m.menuBar.h);

    int dx = m.menuBar.x, dy = m.menuBar.y;
    int up = PlanesGeometry::Y_OFFSET;

    auto& settings = opt.settings;
    auto& help = opt.help;
    auto& about = opt.about;

    background.blendFrom(settings,
                         m.settings.x - dx,
                         m.settings.y - dy + up,
                         0, 0, BUTTON_NORMAL);
    background.blendFrom(help,
                         m.help.x - dx,
                         m.help.y - dy + up,
                         0, 0, BUTTON_NORMAL);
    background.blendFrom(about,
                         m.about.x - dx,
                         m.about.y - dy + up,
                         0, 0, BUTTON_NORMAL);

    left.copyFrom(background, 0, 0, 0, 0, left.width(), left.height());
    right.copyFrom(background, 0, 0,
                   static_cast<int>(left.width()), 0, right.width(), left.height());
}

void FlyoutView::initializeScene() {
    auto& m = model_;
    auto& scene = resources_.scene;
    unsigned maxVal = std::max(m.body.w, m.body.h) + 52;
    scene.renderBuffer.create(maxVal, maxVal);
    scene.renderBuffer.clear({0, 0, 0, 0});

    scene.planesGeo.create(static_cast<float>(m.menuBar.w), static_cast<float>(m.body.w), static_cast<float>(m.body.h),
                           0.0f);
    drawMenuBarMaterials();
    scene.planes.create(scene.planesGeo, {scene.planeLeftMat, scene.planeRightMat, scene.planeBodyMat});
    scene.planesGeo.offset(0.0f, 0.0f, -2.9f); // correct Z

    scene.planes.setParent(scene.nullObj);
    scene.planes.geometry().update();

    scene.planesGeo.setMaterialIndex(0, 0); // planeLeftMat
    scene.planesGeo.setMaterialIndex(1, 1); // planeRightMat
    scene.planesGeo.setMaterialIndex(2, 2); // planeBodyMat

    scene.circleGeo.create(m.circle.w, m.circle.h);
    scene.progressBackgroundGeo.create(m.progressBackground.w, m.progressBackground.h);
    scene.progressGeo.create(m.progress.w, m.progress.h);
    scene.lockGeo.create(m.lock.w, m.lock.h);
    scene.keyboardGeo.create(m.keyboard.w, m.keyboard.h);
    scene.mouseGeo.create(m.mouse.w, m.mouse.h);

    scene.circle.create(scene.circleGeo, scene.circleMat);
    scene.progressBackground.create(scene.progressBackgroundGeo, scene.progressBackgroundMat);
    scene.progress.create(scene.progressGeo, scene.progressMat);
    scene.lock.create(scene.lockGeo, {scene.lockOpenMat, scene.lockClosedMat,
                                      scene.lockCancelMat, scene.lockDisabledMat, scene.lockBufferMat});

    scene.keyboard.create(scene.keyboardGeo, {scene.keyboardEnabledMat,
                                              scene.keyboardDisabledMat});
    scene.mouse.create(scene.mouseGeo, {scene.mouseEnabledMat, scene.mouseDisabledMat});
    updateButtonMaterial();

    scene.circle.setParent(scene.nullObj);
    scene.progressBackground.setParent(scene.nullObj);
    scene.progress.setParent(scene.nullObj);
    scene.lock.setParent(scene.nullObj);
    scene.keyboard.setParent(scene.nullObj);
    scene.mouse.setParent(scene.nullObj);

    unsigned centerX = scene.renderBuffer.width() / 2;
    unsigned centerY = scene.renderBuffer.height() / 2;
    scene.nullObj.setPosition0(static_cast<float>(centerX), static_cast<float>(centerY) + 0.5f);

    scene.objects.add(scene.nullObj);
    scene.objects.add(scene.mouse);
    scene.objects.add(scene.keyboard);
    scene.objects.add(scene.lock);
    scene.objects.add(scene.progress);
    scene.objects.add(scene.progressBackground);
    scene.objects.add(scene.circle);
    scene.objects.add(scene.planes);

    unsigned w2 = m.body.w / 2;
    unsigned h = m.body.h - PlanesGeometry::Y_OFFSET;
    unsigned h2 = h / 2;
    scene.viewFrame = Rectangle::create(static_cast<int>(centerX - w2), static_cast<int>(centerY - h2),
                                        static_cast<int>(m.body.w), static_cast<int>(h));
}

void FlyoutView::checkImageSizes() {
#ifdef _DEBUG
    auto checkSize = [](auto& bitmap, auto& component) {
        return bitmap.width() == component.w && bitmap.height() == component.h;
    };

    const auto& m = model_;

    auto& title = resources_.title;
    assert(checkSize(title.close, m.close));
    assert(checkSize(title.menu, m.menu));
    assert(checkSize(title.background, m.titleBar));

    auto& opt = resources_.menuBar;
    assert(checkSize(opt.settings, m.settings));
    assert(checkSize(opt.help, m.help));
    assert(checkSize(opt.about, m.about));

    auto& body = resources_.body;
    assert(checkSize(body.progressFull, m.progress));

    auto& scene = resources_.scene;
    assert(checkSize(scene.planeBodyMat.bitmap, m.body));
    assert(checkSize(scene.circleMat.bitmap, m.circle));
    assert(checkSize(scene.progressBackgroundMat.bitmap, m.progressBackground));
    assert(checkSize(scene.keyboardEnabledMat.bitmap, m.keyboard));
    assert(checkSize(scene.keyboardDisabledMat.bitmap, m.keyboard));
    assert(checkSize(scene.mouseEnabledMat.bitmap, m.mouse));
    assert(checkSize(scene.mouseDisabledMat.bitmap, m.mouse));
    assert(checkSize(scene.lockOpenMat.bitmap, m.lock));
    assert(checkSize(scene.lockClosedMat.bitmap, m.lock));
    assert(checkSize(scene.lockDisabledMat.bitmap, m.lock));
    assert(checkSize(scene.lockCancelMat.bitmap, m.lock));
#endif // _DEBUG
}

void FlyoutView::drawProgress(Bitmap& buffer, float progress) const {
    Bitmap mask;
    mask.createAs(buffer);
    BitmapContext ctx(mask);
    unsigned w = buffer.width();
    unsigned h = buffer.height();
    int xc = static_cast<int>(w / 2);
    int yc = static_cast<int>(h / 2);
    float r = static_cast<float>(w) / 2.0f;
    ctx.clear({255, 255, 255, 255});
    ctx.beginPath();
    ctx.moveTo(xc, 0);
    ctx.lineTo(xc, yc);

    float angle = 360.0f * progress / 100.0f;
    const auto PI = static_cast<float>(std::numbers::pi);
    float a = angle * PI / 180.0f;
    ctx.lineTo(static_cast<float>(xc) + r * sin(a), static_cast<float>(yc) - r * cos(a));

    if (angle < 90) {
        ctx.lineTo(w, 0u);
    }
    if (angle < 180) {
        ctx.lineTo(w, h);
    }
    if (angle < 270) {
        ctx.lineTo(0u, h);
    }
    ctx.lineTo(0, 0);
    ctx.closePath();
    ctx.fillColor = Color(0, 0, 0);
    ctx.fill();

    buffer.blitFrom(resources_.body.progressFull);
    BitmapUtils::applyMask(buffer, mask);
}

void FlyoutView::checkAction(AnimationAction actionBefore) {
    if (actionBefore == AnimationAction::UNLOCK) {
        model_.guard.appEvent = AppEvent(AppEvent::UNLOCK);
        model_.guard.initialize();
        setGuardActiveState(UnlockGuardActiveState::ACTIVE);
    } else if (actionBefore == AnimationAction::EXIT) {
        model_.guard.appEvent = AppEvent(AppEvent::EXIT);
        model_.guard.initialize();
        setGuardActiveState(UnlockGuardActiveState::ACTIVE);
    }
}

bool FlyoutView::needsUpdate() {
    bool result = needsUpdate_;
    needsUpdate_ = false; // reset the flag
    return result;
}

void FlyoutView::draw() {
    draw(buffer_, !model_.visible(), isMenuBarVisible() ? 1.0f : 0.0f);
}

void FlyoutView::draw(Bitmap& buffer, bool isHidden, float menuBar /*= 0*/, bool showGuard/* = true*/) {
    if (isHidden) {
        buffer.clear({0, 0, 0, 0});
        return;
    }

    renderBackground(menuBar, FlyoutLayers::all(), true);
    drawBackground(buffer);

    model_.menuBar.visible = isFloatEquals(menuBar, 1.0f);
    updateOptButtons(buffer);


    //
    // title
    //
    auto title = model_.titleBar;
    buffer.copyFrom(resources_.title.background, title.x, title.y);

    //
    // title buttons
    //
    auto close = model_.close;
    auto settings = model_.menu;
    std::uint8_t closeAlpha = close.pressed ? 100 : close.hover ? 160 : 120;
    std::uint8_t settingsAlpha = settings.pressed ? 100 : settings.hover ? 160 : 120;
    buffer.blendFrom(resources_.title.close, close.x, close.y, 0, 0, closeAlpha);
    buffer.blendFrom(resources_.title.menu, settings.x, settings.y, 0, 0, settingsAlpha);

    if (showGuard && model_.guard.visible()) {
        guard_.draw(buffer);
    }
}

void FlyoutView::updateOptButtons(Bitmap& buffer) {
    auto& m = model_;
    auto& opt = m.menuBar;
    m.settings.visible = opt.visible;
    m.help.visible = opt.visible;
    m.about.visible = opt.visible;

    if (opt.visible) {
        BitmapContext ctx(buffer);
        ctx.color = model_.menuBarBackground;
        ctx.rect(opt.x, opt.y,
                 static_cast<int>(opt.w), static_cast<int>(opt.h) - PlanesGeometry::Y_OFFSET);

        ctx.color = Color(50, 50, 50);

        std::uint8_t settingsAlpha = BUTTON_NORMAL;
        std::uint8_t helpAlpha = BUTTON_NORMAL;
        std::uint8_t aboutAlpha = BUTTON_NORMAL;

        if (m.settings.pressed) {
            settingsAlpha = BUTTON_PRESSED;
        } else if (m.help.pressed) {
            helpAlpha = BUTTON_PRESSED;
        } else if (m.about.pressed) {
            aboutAlpha = BUTTON_PRESSED;
        } else if (m.settings.hover) {
            settingsAlpha = BUTTON_HOVER;
            ctx.rect(m.settings.x, m.settings.y,
                     static_cast<int>(m.settings.w), static_cast<int>(m.settings.h));
        } else if (m.help.hover) {
            helpAlpha = BUTTON_HOVER;
            ctx.rect(m.help.x, m.help.y,
                     static_cast<int>(m.help.w), static_cast<int>(m.help.h));
        } else if (m.about.hover) {
            aboutAlpha = BUTTON_HOVER;
            ctx.rect(m.about.x, m.about.y,
                     static_cast<int>(m.about.w), static_cast<int>(m.about.h));
        }

        auto& settings = resources_.menuBar.settings;
        auto& help = resources_.menuBar.help;
        auto& about = resources_.menuBar.about;
        buffer.blendFrom(settings, m.settings.x, m.settings.y, 0, 0, settingsAlpha);
        buffer.blendFrom(help, m.help.x, m.help.y, 0, 0, helpAlpha);
        buffer.blendFrom(about, m.about.x, m.about.y, 0, 0, aboutAlpha);
    }
}

unsigned FlyoutView::getLockMaterialIndex() {
    switch (model_.lockState()) {
        case LockState::DISABLED:
            return FlyoutResources::LOCK_DISABLED_MAT;
        case LockState::PRE_LOCKED: // not locked yet
        case LockState::READY:
            return FlyoutResources::LOCK_OPEN_MAT;
        case LockState::PRE_COUNTDOWN: // not counting down yet
        case LockState::COUNTDOWN:
            return FlyoutResources::LOCK_CANCEL_MAT;
        case LockState::PRE_READY:  // not ready yet
        case LockState::LOCKED:
            return FlyoutResources::LOCK_CLOSED_MAT;
        default:
            assert(false);
            return FlyoutResources::LOCK_OPEN_MAT; // default icon
    }
}

void FlyoutView::renderBackground(float value, FlyoutLayers layers, bool useCache) {
    auto&& scene = resources_.scene;
    auto&& buf = scene.renderBuffer;
    buf.clear({0, 0, 0, 0});

    scene.planesGeo.update(value);
    scene.planesGeo.offset(0, 0, -3); // correct Z position
    if (useCache) {
        Bitmap *drawnLayers = nullptr;
        if (isFloatEquals(value, 0.0f)) {
            drawnLayers = &resources_.body.menuClosed.backLayers;
        } else if (isFloatEquals(value, 1.0f)) {
            drawnLayers = &resources_.body.menuOpen.backLayers;
        }

        if (drawnLayers) {
            // skip layers below
            layers.planes = false;
            layers.circle = false;
            layers.progressBackground = false;

            auto& rc = scene.viewFrame;
            buf.copyFrom(*drawnLayers, rc.left, rc.top, model_.body.x, model_.body.y,
                         Rectangle::width(rc), Rectangle::height(rc));
        }
    }

    scene.planes.visible = layers.planes;

    auto angle = scene.planesGeo.angle();
    auto center = scene.planesGeo.center();
    float yValue = -3.0f * value;
    float zValue = EASY_IN(value) * 1.5f;

    //
    // Some Z-offset values
    //
    float circleZOffset(2),
            ringZOffset(-5),
            progressZOffset(-10),
            lockZOffset(-11);

    scene.circle.visible = layers.circle;
    if (layers.circle) {
        scene.circleGeo.setSize(model_.circle.w, model_.circle.h);
        scene.circleGeo.offset(0, yValue, circleZOffset * zValue);
        scene.circleGeo.offset(model_.circle.offset3.x, model_.circle.offset3.y, model_.circle.offset3.z);
        scene.circle.setRotY(angle);
        scene.circle.setPosition(center[0], center[1], center[2]);
    }

    scene.progressBackground.visible = layers.progressBackground;
    if (layers.progressBackground) {
        scene.progressBackgroundGeo.setSize(model_.progressBackground.w, model_.progressBackground.h);
        scene.progressBackgroundGeo.offset(0, yValue, ringZOffset * zValue);
        scene.progressBackgroundGeo.offset(model_.progressBackground.offset3.x, model_.progressBackground.offset3.y,
                                           model_.progressBackground.offset3.z);
        scene.progressBackground.setRotY(angle);
        scene.progressBackground.setPosition(center[0], center[1], center[2]);

        int alpha = 255 - std::lround(75.0f * value);
        alpha = std::clamp(alpha, 0, 255);
        scene.progressBackgroundMat.alpha = static_cast<std::uint8_t>(alpha);
    }

    scene.progress.visible = layers.progress;
    if (layers.progress) {
        scene.progressGeo.setSize(model_.progress.w, model_.progress.h);
        scene.progressGeo.offset(0, yValue, progressZOffset * zValue);
        scene.progressGeo.offset(model_.progress.offset3.x, model_.progress.offset3.y, model_.progress.offset3.z);
        scene.progress.setRotY(angle);
        scene.progress.setPosition(center[0], center[1], center[2]);

        drawProgress(scene.progressMat.bitmap, progressBar_.progress());
        scene.progressMat.alpha = progressBar_.opacity();
    }

    scene.lock.visible = layers.lock;
    if (layers.lock) {
        scene.lockGeo.setSize(model_.lock.w, model_.lock.h);
        scene.lockGeo.offset(0, yValue, lockZOffset * zValue);
        scene.lockGeo.offset(model_.lock.offset3.x, model_.lock.offset3.y, model_.lock.offset3.z);
        scene.lock.setRotY(angle);
        if (layers.lockUpdateMaterial) {
            scene.lockGeo.setMaterialIndex(0, getLockMaterialIndex());
        }
        scene.lock.setPosition(center[0], center[1], center[2]);
    }

    scene.keyboard.visible = layers.keyboard;
    if (layers.keyboard) {
        scene.keyboardGeo.setSize(model_.keyboard.w, model_.keyboard.h);
        scene.keyboardGeo.offset(0, yValue, -18 * zValue);
        scene.keyboardGeo.offset(model_.keyboard.offset3.x, model_.keyboard.offset3.y, model_.keyboard.offset3.z);
        scene.keyboard.setRotY(angle);
        scene.keyboard.setPosition(center[0] - 50.5f, center[1] + 37.8f, center[2]);
        scene.keyboardEnabledMat.alpha = buttonOpacity_.opacity();
        scene.keyboardDisabledMat.alpha = buttonOpacity_.opacity();
    }

    scene.mouse.visible = layers.mouse;
    if (layers.mouse) {
        scene.mouseGeo.setSize(model_.mouse.w, model_.mouse.h);
        scene.mouseGeo.offset(0, yValue, -18 * zValue);
        scene.mouseGeo.offset(model_.mouse.offset3.x, model_.mouse.offset3.y, model_.mouse.offset3.z);
        scene.mouse.setRotY(angle);
        scene.mouse.setPosition(center[0] + 50.5f, center[1] + 37.8f, center[2]);
        scene.mouseEnabledMat.alpha = buttonOpacity_.opacity();
        scene.mouseDisabledMat.alpha = buttonOpacity_.opacity();
    }

    scene.renderer.render(scene.objects);
}

void FlyoutView::drawBackground(Bitmap& buffer) const {
    auto& rc = resources_.scene.viewFrame;
    buffer.copyFrom(resources_.scene.renderBuffer, model_.body.x, model_.body.y, rc.left, rc.top,
                    Rectangle::width(rc), Rectangle::height(rc));
}

void FlyoutView::setForeground(HWND hWnd) {
    assert(hWnd);
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    SetForegroundWindow(hWnd);
}

void FlyoutView::setGuardActiveState(UnlockGuardActiveState state) {
    model_.guard.setActiveState(state);
}

void FlyoutView::resetGuard() {
    if (model_.guard.visible()) {
        stopCurrentAnimation();
        model_.guard.reset();
        setGuardActiveState(UnlockGuardActiveState::INACTIVE);
        draw();
    }
}

void FlyoutView::updateButtonMaterial() {
    auto& sc = resources_.scene;
    sc.keyboardGeo.setMaterialIndex(0, model_.keyboard.checked ?
                                       FlyoutResources::KEYBOARD_ENABLED_MAT : FlyoutResources::KEYBOARD_DISABLED_MAT);
    sc.mouseGeo.setMaterialIndex(0, model_.mouse.checked ?
                                    FlyoutResources::MOUSE_ENABLED_MAT : FlyoutResources::MOUSE_DISABLED_MAT);
}

bool FlyoutView::updateButtonOpacity() {
    if (isAnimationActive()) {
        return false;
    }

    bool disabled = isMenuBarVisible() || model_.locked();
    if (buttonOpacity_.update(model_.buttonHover, disabled)) {
        draw();

        AppTimer::setAnimationActiveFor(1000);
        return true;
    }
    return false;
}

void FlyoutView::runLocking(bool start) {
    if (start) {
        assert(model_.lockState() == LockState::COUNTDOWN);
        progressBar_.start();
        model_.lockCancel = true;
    } else {
        assert(model_.lockState() == LockState::PRE_LOCKED);
        progressBar_.reset();
        model_.lockCancel = false;
    }
}

void FlyoutView::setScenePlaneLockVisible(bool visible) {
    resources_.scene.lock.visible = visible;
}

bool FlyoutView::isProtected() {
    return SettingsData::instance().preventUnlockingInput.value();
}

} // namespace litelockr
