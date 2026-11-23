#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

class $modify(BuffEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        // Buat tombol "BUFF" di sisi kiri-atas editor
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        menu->setID("auto-buff-menu"_spr);

        auto spr = ButtonSprite::create("BUFF", "goldFont.fnt", "GJ_button_06.png", 0.7f);
        spr->setScale(0.65f);

        auto btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(BuffEditorUI::onBuffClicked)
        );
        btn->setPosition({ 55.f, winSize.height - 95.f });
        btn->setID("buff-button"_spr);

        menu->addChild(btn);
        this->addChild(menu, 100);

        return true;
    }

    void onBuffClicked(CCObject*) {
        auto objs = this->getSelectedObjects();
        if (!objs || objs->count() == 0) {
            Notification::create("Select minimal 1 objek dulu!", NotificationIcon::Warning)->show();
            return;
        }

        // Ambil setting FPS dari Geode Mod Settings
        int64_t targetFps = Mod::get()->getSettingValue<int64_t>("target-fps");
        if (targetFps < 60) targetFps = 60;

        // Kecepatan dasar 1x GD ~ 311.58 unit/detik
        // Jarak 1 frame = 311.58 / targetFps
        float frameWindow = 311.58f / static_cast<float>(targetFps);

        if (objs->count() == 1) {
            // Mode 1 Objek (misal single spike): perbesar skala sedikit sesuai ketatnya FPS
            auto obj = static_cast<GameObject*>(objs->objectAtIndex(0));
            float scaleBoost = 1.0f + (0.15f * (static_cast<float>(targetFps) / 240.0f));
            obj->updateCustomScaleX(obj->m_scaleX * scaleBoost);
            obj->updateCustomScaleY(obj->m_scaleY * scaleBoost);
        } else {
            // Mode Multi Objek (misal corridor / gap atas-bawah):
            // Hitung titik tengah Y, lalu geser objek saling mendekat
            float sumY = 0.f;
            for (auto* obj : CCArrayExt<GameObject*>(objs)) {
                sumY += obj->getPositionY();
            }
            float centerY = sumY / objs->count();

            // Semakin tinggi FPS, semakin jauh geserannya menuju titik tengah
            float tightenAmount = (30.f - frameWindow) * 0.15f;
            if (tightenAmount < 1.0f) tightenAmount = 1.0f;

            for (auto* obj : CCArrayExt<GameObject*>(objs)) {
                float y = obj->getPositionY();
                if (y > centerY) {
                    this->moveObject(obj, { 0.f, -tightenAmount });
                } else if (y < centerY) {
                    this->moveObject(obj, { 0.f, tightenAmount });
                }
            }
        }

        Notification::create(
            fmt::format("Buffed! Target: {} FPS (~{:.2f} unit/frame)", targetFps, frameWindow),
            NotificationIcon::Success
        )->show();
    }
};
