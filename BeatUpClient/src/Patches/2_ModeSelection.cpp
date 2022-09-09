#include "2_ModeSelection.hpp"
#include "../global.hpp"
#include <GlobalNamespace/JoiningLobbyViewController.hpp>
#include <GlobalNamespace/LoadingControl.hpp>
#include <GlobalNamespace/MultiplayerModeSelectionFlowCoordinator.hpp>
#include <GlobalNamespace/MultiplayerStatusData.hpp>
#include <GlobalNamespace/MultiplayerUnavailableReason.hpp>
#include <GlobalNamespace/MultiplayerUnavailableReasonMethods.hpp>
#include <GlobalNamespace/TimeExtensions.hpp>
#include <HMUI/ViewController_AnimationDirection.hpp>
#include <HMUI/ViewController_AnimationType.hpp>
#include <HMUI/ButtonSpriteSwap.hpp>
#include <Polyglot/Localization.hpp>
#include <System/DateTime.hpp>
#include <System/Nullable_1.hpp>
#include <System/TimeSpan.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
using namespace BeatUpClient;

static UnityEngine::GameObject *prevLoading = NULL;
static void ShowLoading(UnityEngine::GameObject *loading) {
	if(prevLoading)
		Object_Destroy(prevLoading, 0);
	prevLoading = loading;
	if(loading) {
		Object_SetName(loading, StringW("BeatUpClient_LoadingControl"));
		loading->get_transform()->set_localPosition(UnityEngine::Vector3(0, 12, 0));
		GetComponent<GlobalNamespace::LoadingControl>(loading)->ShowLoading(Polyglot::Localization::Get("LABEL_CHECKING_SERVER_STATUS"));
	}
}

BSC_MAKE_HOOK_MATCH(HMUI::FlowCoordinator::ProvideInitialViewControllers, void, HMUI::FlowCoordinator *self, ::HMUI::ViewController* mainViewController, ::HMUI::ViewController* leftScreenViewController, ::HMUI::ViewController* rightScreenViewController, ::HMUI::ViewController* bottomScreenViewController, ::HMUI::ViewController* topScreenViewController) {
	GlobalNamespace::JoiningLobbyViewController *originalView = il2cpp_utils::try_cast<GlobalNamespace::JoiningLobbyViewController>(mainViewController).value_or(nullptr);
	GlobalNamespace::MultiplayerModeSelectionFlowCoordinator *flowCoordinator = il2cpp_utils::try_cast<GlobalNamespace::MultiplayerModeSelectionFlowCoordinator>(self).value_or(nullptr);
	if(originalView && flowCoordinator) {
		mainViewController = flowCoordinator->multiplayerModeSelectionViewController;
		if(UnityEngine::Transform *buttons = flowCoordinator->multiplayerModeSelectionViewController->get_transform()->Find("Buttons"); buttons)
			GameObject_SetActive(buttons->get_gameObject(), false);
		GameObject_SetActive(flowCoordinator->multiplayerModeSelectionViewController->maintenanceMessageText->get_gameObject(), false);
		flowCoordinator->set_showBackButton(true);
		ShowLoading(Instantiate(originalView->loadingControl->get_gameObject(), mainViewController->get_transform()));
	}
	base(self, mainViewController, leftScreenViewController, rightScreenViewController, bottomScreenViewController, topScreenViewController);
}

// This callback triggers twice if `_multiplayerStatusModel.GetMultiplayerStatusAsync()` was cancelled by pressing the back button
BSC_MAKE_HOOK_MATCH(GlobalNamespace::MainFlowCoordinator::HandleMultiplayerModeSelectionFlowCoordinatorDidFinish, void, GlobalNamespace::MainFlowCoordinator *self, ::GlobalNamespace::MultiplayerModeSelectionFlowCoordinator* multiplayerModeSelectionFlowCoordinator) {
	if(multiplayerModeSelectionFlowCoordinator->get_isActivated())
		base(self, multiplayerModeSelectionFlowCoordinator);
}

// The UI deletes itself at the end of `MultiplayerModeSelectionFlowCoordinator.TryShowModeSelection()` without this
BSC_MAKE_HOOK_OVERLOAD(HMUI::FlowCoordinator, ReplaceTopViewController, void, HMUI::FlowCoordinator *self, ::HMUI::ViewController* viewController, ::System::Action* finishedCallback, ::HMUI::ViewController::AnimationType animationType, ::HMUI::ViewController::AnimationDirection animationDirection) {
	if(!il2cpp_utils::try_cast<GlobalNamespace::MultiplayerModeSelectionViewController>(viewController).value_or(nullptr))
		return base(self, viewController, finishedCallback, animationType, animationDirection);
	self->TopViewControllerWillChange(viewController, viewController, animationType);
	if(finishedCallback)
		finishedCallback->Invoke();
}

static StringW GetMaintenanceMessage(GlobalNamespace::MultiplayerUnavailableReason reason, System::Nullable_1<int64_t> maintenanceWindowEndTime) {
	if(reason == GlobalNamespace::MultiplayerUnavailableReason::MaintenanceMode)
		return Polyglot::Localization::GetFormat(GlobalNamespace::MultiplayerUnavailableReasonMethods::LocalizedKey(reason), ArrayW<Il2CppObject*>({(Il2CppObject*)(GlobalNamespace::TimeExtensions::AsUnixTime(maintenanceWindowEndTime.GetValueOrDefault()) - System::DateTime::get_UtcNow()).ToString("h':'mm")}));
	return Polyglot::Localization::Get(GlobalNamespace::MultiplayerUnavailableReasonMethods::LocalizedKey(reason)) + " (" + GlobalNamespace::MultiplayerUnavailableReasonMethods::ErrorCode(reason) + ")";
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MultiplayerModeSelectionFlowCoordinator::PresentMasterServerUnavailableErrorDialog, void, GlobalNamespace::MultiplayerModeSelectionFlowCoordinator *self, ::GlobalNamespace::MultiplayerUnavailableReason reason, ::System::Exception* exception, ::System::Nullable_1<int64_t> maintenanceWindowEndTime, ::StringW remoteLocalizedMessage) {
	if(!mainFlowCoordinator || mainFlowCoordinator->get_childFlowCoordinator() != self)
		return;
	ShowLoading(NULL);
	TMPro::TextMeshProUGUI *message = self->multiplayerModeSelectionViewController->maintenanceMessageText;
	message->set_text(remoteLocalizedMessage ? remoteLocalizedMessage : GetMaintenanceMessage(reason, maintenanceWindowEndTime));
	message->set_richText(true);
	message->get_transform()->set_localPosition(UnityEngine::Vector3(0, 15, 0));
	GameObject_SetActive(message->get_gameObject(), true);
	self->SetTitle(Polyglot::Localization::Get("LABEL_CONNECTION_ERROR"), HMUI::ViewController::AnimationType::In);
}

// TODO: this is ugly
static inline void SwapSprites(UnityEngine::Sprite **a, UnityEngine::Sprite **b) {
	UnityEngine::Sprite *temp = *a;
	*a = *b;
	*b = temp;
}

std::array<UnityEngine::GameObject*, 4> *BeatUpClient::BeatUpServerUI = NULL;
BSC_MAKE_HOOK_MATCH(GlobalNamespace::MultiplayerModeSelectionViewController::SetData, void, GlobalNamespace::MultiplayerModeSelectionViewController *self, ::GlobalNamespace::MultiplayerStatusData* multiplayerStatusData) {
	static bool createButtonSpriteState = false;
	ShowLoading(NULL);
	self->maintenanceMessageText->set_richText(false);
	self->maintenanceMessageText->get_transform()->set_localPosition(UnityEngine::Vector3(0, -5, 0));
	bool isBeatUp = false;
	if(multiplayerStatusData && multiplayerStatusData->minimumAppVersion)
		isBeatUp = multiplayerStatusData->minimumAppVersion->EndsWith("b2147483647");
	if(BeatUpServerUI)
		for(UnityEngine::GameObject *element : *BeatUpServerUI)
			GameObject_SetActive(element, isBeatUp);
	UnityEngine::Transform *buttons = self->get_transform()->Find("Buttons");
	if(!buttons)
		return base(self, multiplayerStatusData);
	if(createButtonSpriteState != isBeatUp) {
		UnityEngine::Transform *CreateServerButton = buttons->Find("CreateServerButton");
		if(CreateServerButton) {
			HMUI::ButtonSpriteSwap *sprites = GetComponent<HMUI::ButtonSpriteSwap>(CreateServerButton->get_gameObject());
			if(sprites) {
				SwapSprites(&altCreateButtonSprites[0], &sprites->normalStateSprite);
				SwapSprites(&altCreateButtonSprites[1], &sprites->highlightStateSprite);
				SwapSprites(&altCreateButtonSprites[2], &sprites->pressedStateSprite);
				SwapSprites(&altCreateButtonSprites[3], &sprites->disabledStateSprite);
				createButtonSpriteState = isBeatUp;
			}
		}
	}
	GameObject_SetActive(buttons->get_gameObject(), true);
	base(self, multiplayerStatusData);
}
