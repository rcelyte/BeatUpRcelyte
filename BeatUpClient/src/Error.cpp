#include "global.hpp"
#include "Error.hpp"
#include <GlobalNamespace/MainFlowCoordinator.hpp>
#include <GlobalNamespace/SimpleDialogPromptViewController.hpp>
#include <HMUI/ViewController_AnimationDirection.hpp>
#include <Polyglot/Localization.hpp>

static PatchFunc ApplyPatches = []() {
	BeatUpClient::logger->info("BeatUpClient_Error::ApplyPatches()");
	return false;
};

static const char *header = NULL, *message = NULL;
static bool skip = false;
BSC_MAKE_HOOK_MATCH(GlobalNamespace::MainFlowCoordinator::HandleMainMenuViewControllerDidFinish, void, GlobalNamespace::MainFlowCoordinator *self, ::GlobalNamespace::MainMenuViewController* viewController, ::GlobalNamespace::MainMenuViewController::MenuButton subMenuType) {
	if(skip || subMenuType != GlobalNamespace::MainMenuViewController::MenuButton::Multiplayer) {
		base(self, viewController, subMenuType);
		return;
	}
	self->simpleDialogPromptViewController->Init(header, message, Polyglot::Localization::Get("BUTTON_OK"), Delegate_Action([self, viewController](int32_t buttonNumber) {
		skip = true;
		self->DismissViewController(self->simpleDialogPromptViewController, HMUI::ViewController::AnimationDirection::Vertical, Delegate_Action([self, viewController]() {
			self->HandleMainMenuViewControllerDidFinish(viewController, GlobalNamespace::MainMenuViewController::MenuButton::Multiplayer);
		}), false);
	}));
	self->PresentViewController(self->simpleDialogPromptViewController, NULL, HMUI::ViewController::AnimationDirection::Horizontal, false);
}

void BeatUpClient_Error::Init(const char *header, const char *message) {
	::header = header, ::message = message;
	ApplyPatches();
}
