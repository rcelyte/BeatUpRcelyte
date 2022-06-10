extern "C" {
	#define restrict
	#include "packets.h"
	#include "json.h"
}
#include "misc.hpp"
#include <numeric>

#include <GlobalNamespace/AuthenticationToken.hpp>
#include <GlobalNamespace/BasicConnectionRequestHandler.hpp>
#include <GlobalNamespace/BeatmapCharacteristicCollectionSO.hpp>
#include <GlobalNamespace/BeatmapCharacteristicSegmentedControlController.hpp>
#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/BeatmapDifficultySegmentedControlController.hpp>
#include <GlobalNamespace/BeatmapIdentifierNetSerializable.hpp>
#include <GlobalNamespace/BeatmapLevelSelectionMask.hpp>
#include <GlobalNamespace/BoolSettingsController.hpp>
#include <GlobalNamespace/BoolSO.hpp>
#include <GlobalNamespace/ClientCertificateValidator.hpp>
#include <GlobalNamespace/ColorsOverrideSettingsPanelController.hpp>
#include <GlobalNamespace/ConnectedPlayerManager.hpp>
#include <GlobalNamespace/CreateServerFormController.hpp>
#include <GlobalNamespace/CustomDifficultyBeatmap.hpp>
#include <GlobalNamespace/CustomDifficultyBeatmapSet.hpp>
#include <GlobalNamespace/CustomNetworkConfig.hpp>
#include <GlobalNamespace/CustomPreviewBeatmapLevel.hpp>
#include <GlobalNamespace/DnsEndPoint.hpp>
#include <GlobalNamespace/DropdownSettingsController.hpp>
#include <GlobalNamespace/EnterPlayerGuestNameViewController.hpp>
#include <GlobalNamespace/EnvironmentInfoSO.hpp>
#include <GlobalNamespace/FormattedFloatListSettingsController.hpp>
#include <GlobalNamespace/GameLiftConnectionManager.hpp>
#include <GlobalNamespace/GameplayModifiers.hpp>
#include <GlobalNamespace/GameplayServerConfiguration.hpp>
#include <GlobalNamespace/GameServerPlayersTableView.hpp>
#include <GlobalNamespace/GameServerPlayerTableCell.hpp>
#include <GlobalNamespace/IConnectedPlayer.hpp>
#include <GlobalNamespace/IMenuRpcManager.hpp>
#include <GlobalNamespace/IPoolablePacket.hpp>
#include <GlobalNamespace/IPreviewBeatmapLevel.hpp>
#include <GlobalNamespace/JoiningLobbyViewController.hpp>
#include <GlobalNamespace/LevelSelectionNavigationController.hpp>
#include <GlobalNamespace/ListSettingsController.hpp>
#include <GlobalNamespace/LoadingControl.hpp>
#include <GlobalNamespace/LobbyPlayersDataModel.hpp>
#include <GlobalNamespace/LobbySetupViewController.hpp>
#include <GlobalNamespace/MainFlowCoordinator.hpp>
#include <GlobalNamespace/MainSettingsModelSO.hpp>
#include <GlobalNamespace/MainSystemInit.hpp>
#include <GlobalNamespace/MasterServerConnectionManager.hpp>
#include <GlobalNamespace/MenuRpcManager.hpp>
#include <GlobalNamespace/MultiplayerLevelLoader.hpp>
#include <GlobalNamespace/MultiplayerLevelSelectionFlowCoordinator.hpp>
#include <GlobalNamespace/MultiplayerLobbyAvatarManager.hpp>
#include <GlobalNamespace/MultiplayerLocalActivePlayerInGameMenuViewController.hpp>
#include <GlobalNamespace/MultiplayerModeSelectionFlowCoordinator.hpp>
#include <GlobalNamespace/MultiplayerOutroAnimationController.hpp>
#include <GlobalNamespace/MultiplayerSessionManager.hpp>
#include <GlobalNamespace/MultiplayerSettingsPanelController.hpp>
#include <GlobalNamespace/MultiplayerStatusModel.hpp>
#include <GlobalNamespace/MultiplayerUnavailableReason.hpp>
#include <GlobalNamespace/MultiplayerUnavailableReasonMethods.hpp>
#include <GlobalNamespace/NetworkConfigSO.hpp>
#include <GlobalNamespace/NetworkConstants.hpp>
#include <GlobalNamespace/NetworkPacketSerializer_2.hpp>
#include <GlobalNamespace/NetworkPlayerEntitlementChecker.hpp>
#include <GlobalNamespace/PacketPool_1.hpp>
#include <GlobalNamespace/PlatformAuthenticationTokenProvider.hpp>
#include <GlobalNamespace/PreviewDifficultyBeatmap.hpp>
#include <GlobalNamespace/PreviewDifficultyBeatmapSet.hpp>
#include <GlobalNamespace/QuickPlaySetupModel.hpp>
#include <GlobalNamespace/ServerCodeEntryViewController.hpp>
#include <GlobalNamespace/SongPackMask.hpp>
#include <GlobalNamespace/StandardLevelDetailView.hpp>
#include <GlobalNamespace/StepValuePicker.hpp>
#include <GlobalNamespace/StringSO.hpp>
#include <GlobalNamespace/SwitchSettingsController.hpp>
#include <GlobalNamespace/ThreadStaticPacketPool_1.hpp>
#include <GlobalNamespace/TimeExtensions.hpp>
#include <HMUI/ButtonBinder.hpp>
#include <HMUI/CurvedTextMeshPro.hpp>
#include <HMUI/FlowCoordinator.hpp>
#include <HMUI/HoverHintController.hpp>
#include <HMUI/IconSegmentedControl.hpp>
#include <HMUI/ImageView.hpp>
#include <HMUI/InputFieldView.hpp>
#include <HMUI/InputFieldView_InputFieldChanged.hpp>
#include <HMUI/NoTransitionsButton.hpp>
#include <HMUI/ScreenSystem.hpp>
#include <HMUI/SimpleTextDropdown.hpp>
#include <HMUI/TextSegmentedControl.hpp>
#include <HMUI/UIKeyboard.hpp>
#include <HMUI/UIKeyboardKey.hpp>
#include <HMUI/ViewController.hpp>
#include <HMUI/ViewController_AnimationDirection.hpp>
#include <HMUI/ViewController_AnimationType.hpp>
#include <LiteNetLib/NetConnectAcceptPacket.hpp>
#include <LiteNetLib/NetConstants.hpp>
#include <LiteNetLib/NetPacket.hpp>
#include <LiteNetLib/NetPeer.hpp>
#include <LiteNetLib/ReliableChannel.hpp>
#include <LiteNetLib/Utils/INetSerializable.hpp>
#include <LiteNetLib/Utils/NetDataReader.hpp>
#include <LiteNetLib/Utils/NetDataWriter.hpp>
#include <Polyglot/GoogleDriveDownloadFormat.hpp>
#include <Polyglot/Localization.hpp>
#include <Polyglot/LocalizationImporter.hpp>
#include <Polyglot/LocalizedTextMeshProUGUI.hpp>
#include <System/Action.hpp>
#include <System/Action_1.hpp>
#include <System/Action_2.hpp>
#include <System/Action_3.hpp>
#include <System/Action_4.hpp>
#include <System/Array.hpp>
#include <System/Collections/Generic/Dictionary_2.hpp>
#include <System/Collections/Generic/InsertionBehavior.hpp>
#include <System/Collections/Generic/List_1.hpp>
#include <System/Nullable_1.hpp>
#include <System/Threading/Tasks/Task_1.hpp>
#include <System/TimeSpan.hpp>
#include <UnityEngine/Component.hpp>
#include <UnityEngine/Events/BaseInvokableCall.hpp>
#include <UnityEngine/Events/InvokableCallList.hpp>
#include <UnityEngine/Events/UnityAction.hpp>
#include <UnityEngine/Events/UnityAction_1.hpp>
#include <UnityEngine/Events/UnityAction_2.hpp>
#include <UnityEngine/Events/UnityEventBase.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/RectTransform.hpp>
#include <UnityEngine/SceneManagement/SceneManager.hpp>
#include <UnityEngine/Sprite.hpp>
#include <UnityEngine/Transform.hpp>
#include <UnityEngine/UI/Button.hpp>
#include <UnityEngine/UI/Button_ButtonClickedEvent.hpp>
#include <UnityEngine/UI/ContentSizeFitter.hpp>
#include <UnityEngine/UI/HorizontalLayoutGroup.hpp>
#include <UnityEngine/UI/Toggle.hpp>
#include <UnityEngine/UI/Toggle_ToggleEvent.hpp>
#include <UnityEngine/UI/VerticalLayoutGroup.hpp>
#include <VRUIControls/VRInputModule.hpp>
#include <Zenject/ConcreteIdBinderNonGeneric.hpp>
#include <Zenject/DiContainer.hpp>

#define DL_EXPORT __attribute__((visibility("default")))

#ifndef DISABLE_DOWNLOADER
#error Direct downloads not implemented.
#endif

using namespace std::literals::string_view_literals;

// static Configuration config({""});
static Logger *logger = NULL;

// C wrapper for logging with beatsaber-hook
extern "C" void LogString(const char *str) {
	logger->info("%s", str);
}

static constexpr const uint64_t MaxDownloadSize = 268435456;
// static constexpr const uint64_t MaxUnzippedSize = 268435456;
static constexpr const uint32_t BeatUpConnectInfo_Size = 6;

namespace Config {
	static const std::pair<std::string, std::string> DefaultServer = {"master.battletrains.org", ""};
	static float CountdownDuration = 5;
	static bool SkipResults = false;
	static bool PerPlayerDifficulty = false;
	static bool PerPlayerModifiers = false;
	// static bool UnreliableState = false;
	static bool DirectDownloads = true;
	static bool AllowModchartDownloads = false;
	static std::vector<std::pair<std::string, std::string>> Servers = {
		DefaultServer,
		{"master.beattogether.systems", "http://master.beattogether.systems/status"},
	};
	static bool EnableOfficialServer = false;
}
static constexpr BeatUpConnectInfo DefaultConnectInfo() {
	return {
		.windowSize = LiteNetLib::NetConstants::DefaultWindowSize,
		.countdownDuration = 20,
		.directDownloads = false,
		.skipResults = false,
		.perPlayerDifficulty = false,
		.perPlayerModifiers = false,
	};
}
static BeatUpConnectInfo connectInfo = DefaultConnectInfo();
static bool clientActive = false;
static bool downloadPending = false;
static UnityEngine::GameObject *infoText = NULL;

BSC_ICALL(Object_Destroy, "UnityEngine.Object::Destroy", void, UnityEngine::Object *obj, float t)
BSC_ICALL(Object_SetName, "UnityEngine.Object::SetName", void, UnityEngine::Object *obj, Il2CppString *name)
BSC_ICALL(Object_DontDestroyOnLoad, "UnityEngine.Object::DontDestroyOnLoad", void, UnityEngine::Object *target)
BSC_ICALL(Object_Internal_CloneSingle, "UnityEngine.Object::Internal_CloneSingle", UnityEngine::Object*, UnityEngine::Object *data)
BSC_ICALL(Object_Internal_CloneSingleWithParent, "UnityEngine.Object::Internal_CloneSingleWithParent", UnityEngine::Object*, UnityEngine::Object *data, UnityEngine::Transform *parent, bool worldPositionStays)
BSC_ICALL(GameObject_SetActive, "UnityEngine.GameObject::SetActive", void, UnityEngine::GameObject *self, bool value)
BSC_ICALL(GameObject_GetComponent, "UnityEngine.GameObject::GetComponent", UnityEngine::Component*, UnityEngine::GameObject *self, System::Type *type)
BSC_ICALL(GameObject_GetComponentsInternal, "UnityEngine.GameObject::GetComponentsInternal", System::Array*, UnityEngine::GameObject *self, System::Type *type, bool useSearchTypeAsArrayReturnType, bool recursive, bool includeInactive, bool reverse, Il2CppObject *resultList)
BSC_ICALL(GameObject_GetComponentInChildren, "UnityEngine.GameObject::GetComponentInChildren", UnityEngine::Component*, UnityEngine::GameObject *self, System::Type *type, bool includeInactive)

template<typename T> static inline T *GetComponent(UnityEngine::GameObject *gameObject) {
	return il2cpp_utils::cast<T>(GameObject_GetComponent(gameObject, csTypeOf(T*)));
}
template<typename T> static inline T *TryGetComponent(UnityEngine::GameObject *gameObject) {
	return il2cpp_utils::try_cast<T>(GameObject_GetComponent(gameObject, csTypeOf(T*))).value_or(nullptr);
}

template<class T, class R, class... TArgs> T MakeDelegate(R (*ptr)(TArgs...)) {
	return il2cpp_utils::MakeDelegate<T>((std::function<R(TArgs...)>)ptr);
}
template<class T, class L> T MakeDelegate(L &&ptr) {
	return il2cpp_utils::MakeDelegate<T>(std::function(ptr));
}
template<class T, class U, class R, class... TArgs> T MakeDelegate(R (U::*ptr)(TArgs...), U *instance) {
	return il2cpp_utils::MakeDelegate<T>((std::function<R(TArgs...)>)[ptr, instance](TArgs... args){return (instance->*ptr)(args...);});
}

static bool String_eq_view(const LongString *str, std::string_view cmp) {
	return str->length == cmp.length() && memcmp(str->data, &cmp[0], str->length) == 0;
}

template<class T = String> static T new_String(std::string_view cstr) {
	T out = {
		.length = uint32_t(cstr.length()),
		.isNull = false,
	};
	if(out.length > lengthof(out.data))
		out.length = lengthof(out.data);
	memcpy(out.data, &cstr[0], out.length);
	return out;
}

template<class T = String> static T new_String(Il2CppString *str) {
	return new_String<T>(to_utf8(csstrtostr(str)));
}

// public class MemorySpriteLoader : ISpriteAsyncLoader {

struct PlayerCell {
	UnityEngine::RectTransform *transform;
	private:
	UnityEngine::UI::Image *background;
	UnityEngine::UI::Image *foreground;
	UnityEngine::Color backgroundColor;
	LoadProgress data;
	public:
	void SetBar(UnityEngine::RectTransform *transform, UnityEngine::UI::Image *background, UnityEngine::Color backgroundColor) {
		this->transform = transform;
		this->background = background;
		this->backgroundColor = backgroundColor;
		foreground = GetComponent<HMUI::ImageView>(transform->get_gameObject());
		Refresh();
	}
	void UpdateData(LoadProgress data, bool replace = false) {
		if(replace)
			data.sequence = this->data.sequence;
		else if(data.sequence < this->data.sequence)
			return;
		SetData(data);
	}
	void SetData(LoadProgress data) {
		this->data = data;
		if(transform)
			Refresh();
	}
	private:
	void Refresh() {
		float delta = (65535u - data.progress) * -104 / 65534.f;
		transform->set_sizeDelta(UnityEngine::Vector2(delta, transform->get_sizeDelta().y));
		transform->set_localPosition(UnityEngine::Vector3(delta * (data.state == LoadState_Exporting ? .5f : -.5f), 0, 0));
		switch(data.state) {
			case LoadState_Exporting: foreground->set_color(UnityEngine::Color(0.9130435f, 1, 0.1521739f, 0.5434783f)); break;
			case LoadState_Downloading: foreground->set_color(UnityEngine::Color(0.4782609f, 0.6956522f, 0.02173913f, 0.5434783f)); break;
			default: foreground->set_color(UnityEngine::Color(0, 0, 0, 0));
		};
		switch(data.state) {
			case LoadState_Failed: background->set_color(UnityEngine::Color(0.7173913f, 0.2608696f, 0.02173913f, 0.7490196f)); break;
			case LoadState_Exporting: background->set_color(UnityEngine::Color(0.2608696f, 0.7173913f, 0.02173913f, 0.2509804f)); break;
			case LoadState_Downloading: background->set_color(UnityEngine::Color(0.2608696f, 0.7173913f, 0.02173913f, 0.2509804f)); break;
			case LoadState_Done: background->set_color(UnityEngine::Color(0.2608696f, 0.7173913f, 0.02173913f, 0.7490196f)); break;
			default: background->set_color(backgroundColor);
		};
		foreground->set_enabled(data.progress >= 1);
		background->set_enabled(true);
	}
};

static char *TryReadText(const char *path) {
	FILE *file = fopen(path, "rb");
	if(!file)
		return NULL;
	fseek(file, 0, SEEK_END);
	size_t data_len = ftell(file);
	char *data = (char*)malloc(data_len + 1);
	if(data) {
		fseek(file, 0, SEEK_SET);
		if(fread(data, 1, data_len, file) == data_len)
			data[data_len - 1] = 0;
		else
			free(data), data = NULL;
	}
	fclose(file);
	return data;
}

static RecommendPreview new_RecommendPreview(GlobalNamespace::CustomPreviewBeatmapLevel *previewBeatmapLevel, std::vector<std::string_view> *requirements, std::vector<std::string_view> *suggestions) {
	RecommendPreview out = {
		.base = {
			.levelId = new_String<LongString>(previewBeatmapLevel->get_levelID()),
			.songName = new_String<LongString>(previewBeatmapLevel->get_songName()),
			.songSubName = new_String<LongString>(previewBeatmapLevel->get_songSubName()),
			.songAuthorName = new_String<LongString>(previewBeatmapLevel->get_songAuthorName()),
			.levelAuthorName = new_String<LongString>(previewBeatmapLevel->get_levelAuthorName()),
			.beatsPerMinute = previewBeatmapLevel->get_beatsPerMinute(),
			.songTimeOffset = previewBeatmapLevel->get_songTimeOffset(),
			.shuffle = previewBeatmapLevel->get_shuffle(),
			.shufflePeriod = previewBeatmapLevel->get_shufflePeriod(),
			.previewStartTime = previewBeatmapLevel->get_previewStartTime(),
			.previewDuration = previewBeatmapLevel->get_previewDuration(),
			.songDuration = previewBeatmapLevel->get_songDuration(),
			.cover = { // TODO: generate cover
				.length = 0,
			},
		},
		.requirements_len = 0,
		.suggestions_len = 0,
	};
	System::Collections::Generic::IReadOnlyList_1<GlobalNamespace::PreviewDifficultyBeatmapSet*> *sets = previewBeatmapLevel->get_previewDifficultyBeatmapSets();
	out.base.count = ((System::Collections::Generic::IReadOnlyCollection_1<GlobalNamespace::PreviewDifficultyBeatmapSet*>*)sets)->get_Count();
	for(uint32_t i = 0; i < out.base.count; ++i) {
		GlobalNamespace::PreviewDifficultyBeatmapSet *set = sets->get_Item(i);
		PreviewDifficultyBeatmapSet *set_out = &out.base.previewDifficultyBeatmapSets[i];
		set_out->beatmapCharacteristicSerializedName = new_String(set->get_beatmapCharacteristic()->get_serializedName());
		set_out->count = 0;
		for(GlobalNamespace::BeatmapDifficulty diff : set->get_beatmapDifficulties())
			if(set_out->count < lengthof(set_out->difficulties))
				set_out->difficulties[set_out->count++] = diff;
	}
	for(std::string_view str : *requirements)
		out.requirements[out.requirements_len++] = new_String(str);
	for(std::string_view str : *suggestions)
		out.suggestions[out.suggestions_len++] = new_String(str);
	return out;
}

struct PlayerData {
	struct ModifiersWeCareAbout {
		GlobalNamespace::GameplayModifiers::SongSpeed songSpeed = 255;
		bool disappearingArrows;
		bool ghostNotes;
		float notesUniformScale;
		ModifiersWeCareAbout() {}
		ModifiersWeCareAbout(GlobalNamespace::GameplayModifiers *src) {
			songSpeed = src->get_songSpeed();
			disappearingArrows = src->get_disappearingArrows();
			ghostNotes = src->get_ghostNotes();
			notesUniformScale = src->get_notesUniformScale();
		}
		/*static implicit operator MultiplayerConnectedPlayerBeatmapObjectManager.InitData(ModifiersWeCareAbout mods) =>
			new MultiplayerConnectedPlayerBeatmapObjectManager.InitData(mods.disappearingArrows, mods.ghostNotes, mods.notesUniformScale);*/
	};
	uint32_t len = 0;
	RecommendPreview previews[128];
	PlayerCell cells[128];
	ModifiersWeCareAbout modifiers[128];
	ModifiersWeCareAbout lockedModifiers[128];
	void Init(int32_t playerCount) {
		len = playerCount;
		memset(previews, 0, sizeof(previews));
		memset(cells, 0, sizeof(cells));
		for(ModifiersWeCareAbout &e : modifiers)
			e = ModifiersWeCareAbout();
		for(ModifiersWeCareAbout &e : lockedModifiers)
			e = ModifiersWeCareAbout();
	}
	void Reset(int index) {
		cells[index].SetData(LoadProgress{0, LoadState_None, 0});
		lockedModifiers[index] = modifiers[index] = ModifiersWeCareAbout();
	}
	RecommendPreview *ResolvePreview(std::string_view levelId) {
		for(uint32_t i = 0; i < len; ++i)
			if(String_eq_view(&previews[i].base.levelId, levelId))
				return &previews[i];
		return NULL;
	}
	RecommendPreview *SetPreviewFromLocal(int index, GlobalNamespace::CustomPreviewBeatmapLevel *previewBeatmapLevel) {
		static constexpr const uint32_t MaxReqs = std::min(lengthof(RecommendPreview::requirements), lengthof(RecommendPreview::suggestions));
		std::vector<std::string_view> requirements, suggestions;
		std::string path = to_utf8(csstrtostr(previewBeatmapLevel->get_customLevelPath())) + "/Info.dat";
		char *info_dat = TryReadText(path.c_str());
		if(info_dat) {
			const char *key, *it = info_dat; // TODO: figure out how to use RapidJSON without involving `try` blocks or premature termination, to match the C# version
			for(uint32_t key_len; json_iter_object(&it, &key, &key_len);) {
				if(strncmp(key, "_difficultyBeatmapSets", key_len)) {
					it = json_skip_value(it);
					continue;
				}
				while(json_iter_array(&it)) {
					while(json_iter_object(&it, &key, &key_len)) {
						if(strncmp(key, "_difficultyBeatmaps", key_len)) {
							it = json_skip_value(it);
							continue;
						}
						while(json_iter_array(&it)) {
							while(json_iter_object(&it, &key, &key_len)) {
								if(strncmp(key, "_customData", key_len)) {
									it = json_skip_value(it);
									continue;
								}
								while(json_iter_object(&it, &key, &key_len)) {
									std::vector<std::string_view> *out = &suggestions;
									if(strncmp(key, "_requirements", key_len) == 0) {
										out = &requirements;
									} else if(strncmp(key, "_suggestions", key_len)) {
										it = json_skip_value(it);
										continue;
									}
									while(json_iter_array(&it)) {
										const char *str;
										uint32_t str_len;
										it = json_get_string(it, &str, &str_len);
										if(str_len && out->size() < MaxReqs)
											out->push_back(std::string_view(str, str_len));
									}
								}
							}
						}
					}
				}
			}
		}
		std::string levelId = to_utf8(csstrtostr(previewBeatmapLevel->get_levelID()));
		if(requirements.size()) {
			logger->info("%zu requirements for `%s`:", requirements.size(), levelId.c_str());
			for(std::string_view &req : requirements)
				logger->info("    %.*s", (int)req.length(), &req[0]);
		} else {
			logger->info("No requirements for `%s`", levelId.c_str());
		}
		previews[index] = new_RecommendPreview(previewBeatmapLevel, &requirements, &suggestions);
		if(info_dat)
			free(info_dat);
		return &previews[index];
	}
} static playerData;

DEFINE_CLASS_CODEGEN_INTERFACES(BeatUpClient, NetworkPreviewBeatmapLevel, Il2CppObject, classof(GlobalNamespace::IPreviewBeatmapLevel*),
	::NetworkPreviewBeatmapLevel preview;
	DEFINE_CTOR(ctor, () {});
	DEFINE_OVERRIDE_METHOD(StringW, get_levelID, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_levelID>::get(), () {
		return std::string_view(preview.levelId.data, preview.levelId.length);
	});
	DEFINE_OVERRIDE_METHOD(StringW, get_songName, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_songName>::get(), () {
		return std::string_view(preview.songName.data, preview.songName.length);
	});
	DEFINE_OVERRIDE_METHOD(StringW, get_songSubName, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_songSubName>::get(), () {
		return std::string_view(preview.songSubName.data, preview.songSubName.length);
	});
	DEFINE_OVERRIDE_METHOD(StringW, get_songAuthorName, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_songAuthorName>::get(), () {
		return std::string_view(preview.songAuthorName.data, preview.songAuthorName.length);
	});
	DEFINE_OVERRIDE_METHOD(StringW, get_levelAuthorName, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_levelAuthorName>::get(), () {
		return std::string_view(preview.levelAuthorName.data, preview.levelAuthorName.length);
	});
	DEFINE_OVERRIDE_METHOD(float, get_beatsPerMinute, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_beatsPerMinute>::get(), () {
		return preview.beatsPerMinute;
	});
	DEFINE_OVERRIDE_METHOD(float, get_songDuration, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_songDuration>::get(), () {
		return preview.songDuration;
	});
	DEFINE_OVERRIDE_METHOD(float, get_previewStartTime, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_previewStartTime>::get(), () {
		return preview.previewStartTime;
	});
	DEFINE_OVERRIDE_METHOD(float, get_previewDuration, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_previewDuration>::get(), () {
		return preview.previewDuration;
	});
	DEFINE_OVERRIDE_METHOD(ArrayW<GlobalNamespace::PreviewDifficultyBeatmapSet*>, get_previewDifficultyBeatmapSets, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_previewDifficultyBeatmapSets>::get(), () {
		return ArrayW<GlobalNamespace::PreviewDifficultyBeatmapSet*>(); // TODO: implement
	});
	DEFINE_OVERRIDE_METHOD(float, get_songTimeOffset, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_songTimeOffset>::get(), () {
		return preview.songTimeOffset;
	});
	DEFINE_OVERRIDE_METHOD(float, get_shuffle, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_shuffle>::get(), () {
		return preview.shuffle;
	});
	DEFINE_OVERRIDE_METHOD(float, get_shufflePeriod, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_shufflePeriod>::get(), () {
		return preview.shufflePeriod;
	});
	DEFINE_OVERRIDE_METHOD(GlobalNamespace::EnvironmentInfoSO*, get_environmentInfo, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_environmentInfo>::get(), () {
		return NULL;
	});
	DEFINE_OVERRIDE_METHOD(GlobalNamespace::EnvironmentInfoSO*, get_allDirectionsEnvironmentInfo, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::get_allDirectionsEnvironmentInfo>::get(), () {
		return NULL;
	});
	DEFINE_OVERRIDE_METHOD(System::Threading::Tasks::Task_1<UnityEngine::Sprite*>*, GetCoverImageAsync, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPreviewBeatmapLevel::GetCoverImageAsync>::get(), (System::Threading::CancellationToken cancellationToken) {
		return System::Threading::Tasks::Task_1<UnityEngine::Sprite*>::New_ctor(static_cast<UnityEngine::Sprite*>(nullptr)); // TODO: implement
	});
	public:
	void Init(RecommendPreview *from, std::string_view levelId) {
		preview = from ? from->base : ::NetworkPreviewBeatmapLevel{
			.levelId = new_String<LongString>(levelId),
			.songName = String_from("[ERROR]"),
			.songSubName = String_from(""),
			.songAuthorName = String_from("[ERROR]"),
			.levelAuthorName = String_from(""),
			.beatsPerMinute = 0,
			.songTimeOffset = 0,
			.shuffle = 0,
			.shufflePeriod = 0,
			.previewStartTime = 0,
			.previewDuration = 0,
			.songDuration = 0,
			.count = 0,
			.cover = {
				.length = 0,
			},
		};
	}
)

static SetCanShareBeatmap new_SetCanShareBeatmap(std::string_view levelId, uint8_t levelHash[32], uint64_t fileSize, bool canShare = true) {
	SetCanShareBeatmap out = {
		.base = {
			.levelId = new_String<LongString>(levelId),
			.fileSize = fileSize,
		},
		.canShare = canShare,
	};
	memcpy(out.base.levelHash, levelHash, sizeof(out.base.levelHash));
	return out;
}

static constexpr const struct PacketContext packetVersion = {
	.netVersion = LiteNetLib::NetConstants::ProtocolId,
	.protocolVersion = GlobalNamespace::NetworkConstants::kProtocolVersion,
	.beatUpVersion = 1,
	.windowSize = LiteNetLib::NetConstants::DefaultWindowSize,
};

#ifdef DISABLE_POOLABLE
#define BeatUpPacket_Interfaces classof(LiteNetLib::Utils::INetSerializable*)
#define BeatUpPacket_DefineRelease
#else
#error TODO: Method: 0x7301b7a0e8 needs virtual_data: 0x72e7aea020 which requires type: 0x72e647f980 which does not exist!
#define BeatUpPacket_Interfaces (classof(LiteNetLib::Utils::INetSerializable*), classof(GlobalNamespace::IPoolablePacket*))
// can't use `ifdef` inside macro
#define BeatUpPacket_DefineRelease \
	DEFINE_OVERRIDE_METHOD(void, Release, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::IPoolablePacket::Release>::get(), () { \
		GlobalNamespace::ThreadStaticPacketPool_1<BeatUpPacket*>::get_pool()->Release(this); \
	});
#endif
DEFINE_CLASS_CODEGEN_INTERFACES(BeatUpClient, BeatUpPacket, Il2CppObject, BeatUpPacket_Interfaces,
	BeatUpMessage message;
	DEFINE_CTOR(ctor, () {});
	DEFINE_OVERRIDE_METHOD(void, Serialize, il2cpp_utils::il2cpp_type_check::MetadataGetter<&LiteNetLib::Utils::INetSerializable::Serialize>::get(), (LiteNetLib::Utils::NetDataWriter* writer) {
		uint8_t data[65536], *data_end = data;
		size_t len = _pkt_try_write((PacketWriteFunc)_pkt_BeatUpMessage_write, &message, &data_end, endof(data), packetVersion);
		if(writer->autoResize)
			writer->ResizeIfNeed(writer->position + len);
		if(writer->position + len <= writer->data.Length()) {
			memcpy(&writer->data[writer->position], data, len);
			writer->position += len;
		}
	});
	 // INetSerializable.Deserialize() will never be called since we didn't register using NetworkPacketSerializer.RegisterCallback()
	DEFINE_OVERRIDE_METHOD(void, Deserialize, il2cpp_utils::il2cpp_type_check::MetadataGetter<&LiteNetLib::Utils::INetSerializable::Deserialize>::get(), (LiteNetLib::Utils::NetDataReader* reader) {});
	BeatUpPacket_DefineRelease
	public:
	static BeatUpPacket *Obtain();
)
#ifdef DISABLE_POOLABLE
BeatUpClient::BeatUpPacket *BeatUpClient::BeatUpPacket::Obtain() {
	return New_ctor();
}
#else
BeatUpClient::BeatUpPacket *BeatUpClient::BeatUpPacket::Obtain() {
	return GlobalNamespace::ThreadStaticPacketPool_1<BeatUpPacket*>::get_pool()->Obtain();
}
#endif

static LoadProgress new_LoadProgress(LoadState state, uint16_t progress) {
	static uint32_t localSequence = 0;
	return {++localSequence, state, progress};
}

class PacketHandler {
	private: void Deserialize(LiteNetLib::Utils::NetDataReader* reader, int32_t length, GlobalNamespace::IConnectedPlayer *player) {
		if(reader->dataSize > reader->data.Length() || length < 1 || reader->position + length > reader->dataSize)
			return;
		BeatUpMessage message;
		const uint8_t *data = &reader->data[reader->position];
		size_t len = _pkt_try_read((PacketReadFunc)_pkt_BeatUpMessage_read, &message, &data, &reader->data[reader->position + length], packetVersion);
		reader->SkipBytes(length);
		if(len != length)
			return;
		switch(message.type) {
			case BeatUpMessageType_RecommendPreview: playerData.previews[player->get_sortIndex()] = message.recommendPreview; break;
			case BeatUpMessageType_SetCanShareBeatmap: break;
			case BeatUpMessageType_DirectDownloadInfo: {
				RecommendPreview *preview = playerData.ResolvePreview(std::string_view(message.directDownloadInfo.base.levelId.data, message.directDownloadInfo.base.levelId.length));
				if(!connectInfo.directDownloads || !preview || MissingRequirements(preview, true) || message.directDownloadInfo.base.fileSize > MaxDownloadSize)
					break;
				if(downloadPending) {
					message.type = BeatUpMessageType_LoadProgress;
					message.loadProgress = new_LoadProgress(LoadState_Downloading, 0);
					HandleLoadProgress(message.loadProgress, multiplayerSessionManager->get_localPlayer());
					Send(message);
				}
				// downloader = new Downloader(packet, preview);
				break;
			}
			case BeatUpMessageType_LevelFragmentRequest: break;
			case BeatUpMessageType_LevelFragment: break;
			case BeatUpMessageType_LoadProgress: HandleLoadProgress(message.loadProgress, player); break;
			default: logger->info("BAD BEAT UP MESSAGE TYPE");
		}
	}
	static constexpr const GlobalNamespace::MultiplayerSessionManager_MessageType messageType = 101;
	public:
	Zenject::DiContainer *container;
	GlobalNamespace::IMenuRpcManager *rpcManager;
	GlobalNamespace::BeatmapCharacteristicCollectionSO *characteristics;
	GlobalNamespace::MultiplayerSessionManager *multiplayerSessionManager;
	void Init(Zenject::DiContainer *container) {
		PacketHandler::container = container;
		rpcManager = CRASH_UNLESS(il2cpp_utils::try_cast<GlobalNamespace::IMenuRpcManager>(container->TryResolve(csTypeOf(GlobalNamespace::IMenuRpcManager*))));
		characteristics = CRASH_UNLESS(il2cpp_utils::try_cast<GlobalNamespace::BeatmapCharacteristicCollectionSO>(container->TryResolve(csTypeOf(GlobalNamespace::BeatmapCharacteristicCollectionSO*))));
		multiplayerSessionManager = CRASH_UNLESS(il2cpp_utils::try_cast<GlobalNamespace::MultiplayerSessionManager>(container->TryResolve(csTypeOf(GlobalNamespace::IMultiplayerSessionManager*))));

		multiplayerSessionManager->SetLocalPlayerState("modded"sv, true);
		multiplayerSessionManager->packetSerializer->typeRegistry->TryInsert(csTypeOf(BeatUpClient::BeatUpPacket*), messageType, System::Collections::Generic::InsertionBehavior::OverwriteExisting);
		multiplayerSessionManager->packetSerializer->messsageHandlers->TryInsert(messageType, MakeDelegate<System::Action_3<LiteNetLib::Utils::NetDataReader*, int, GlobalNamespace::IConnectedPlayer*>*>(&PacketHandler::Deserialize, this), System::Collections::Generic::InsertionBehavior::OverwriteExisting);

		rpcManager->add_setSelectedBeatmapEvent(MakeDelegate<System::Action_2<StringW, GlobalNamespace::BeatmapIdentifierNetSerializable*>*>(HandleSetSelectedBeatmapEvent));
		rpcManager->add_clearSelectedBeatmapEvent(MakeDelegate<System::Action_1<StringW>*>(HandleClearSelectedBeatmapEvent));
		rpcManager->add_setIsEntitledToLevelEvent(MakeDelegate<System::Action_3<StringW, StringW, GlobalNamespace::EntitlementsStatus>*>(&PacketHandler::HandleSetIsEntitledToLevelRpc, this));
		rpcManager->add_recommendGameplayModifiersEvent(MakeDelegate<System::Action_2<StringW, GlobalNamespace::GameplayModifiers*>*>(&PacketHandler::HandleRecommendModifiers, this));
		rpcManager->add_startedLevelEvent(MakeDelegate<System::Action_4<StringW, GlobalNamespace::BeatmapIdentifierNetSerializable*, GlobalNamespace::GameplayModifiers*, float>*>(HandleLevelStart));
	}
	void Send(BeatUpMessage message) {
		BeatUpClient::BeatUpPacket *packet = BeatUpClient::BeatUpPacket::Obtain();
		packet->message = message;
		multiplayerSessionManager->Send((LiteNetLib::Utils::INetSerializable*)packet);
	}
	static bool MissingRequirements(RecommendPreview *preview, bool download) {
		if(!preview)
			return false; // Entitlement[good]: no preview
		if((preview->requirements_len + preview->suggestions_len) && download && !Config::AllowModchartDownloads)
			return true; // Entitlement[fail]: blocked by `AllowModchartDownloads`
		if(!preview->requirements_len)
			return false; // Entitlement[good]: no requirements
		// if(!haveSongCore)
		return true; // Entitlement[fail]: need SongCore
		/*if(!preview.requirements.All(x => NullableStringHelper.IsNullOrEmpty(x) || SongCore.Collections.capabilities.Contains(x)))
			return true; // Entitlement[fail]: missing requirements
		return false; // Entitlement[good]: have all requirements*/
	}
	GlobalNamespace::IConnectedPlayer *GetPlayer(StringW userId) {
		return (GlobalNamespace::IConnectedPlayer*)multiplayerSessionManager->connectedPlayerManager->GetPlayer(userId);
	}
	private:
	static std::u16string_view selectedLevelId;
	static void HandleSetSelectedBeatmapEvent(StringW userId, GlobalNamespace::BeatmapIdentifierNetSerializable *beatmapId) {
		selectedLevelId = beatmapId ? beatmapId->levelID : "";
		for(uint32_t i = 0; i < playerData.len; ++i)
			playerData.cells[i].UpdateData(LoadProgress{0, LoadState_None, 0}, true);
	}
	static void HandleClearSelectedBeatmapEvent(StringW userId) {
		HandleSetSelectedBeatmapEvent(userId, NULL);
	}
	void HandleSetIsEntitledToLevelRpc(StringW userId, StringW levelId, GlobalNamespace::EntitlementsStatus entitlementStatus) {
		HandleSetIsEntitledToLevel(GetPlayer(userId), levelId, entitlementStatus);
	}
	void HandleRecommendModifiers(StringW userId, GlobalNamespace::GameplayModifiers *gameplayModifiers) {
		if(GlobalNamespace::IConnectedPlayer *player = GetPlayer(userId); player)
			playerData.modifiers[player->get_sortIndex()] = PlayerData::ModifiersWeCareAbout(gameplayModifiers);
	}
	static void HandleLevelStart(StringW userId, GlobalNamespace::BeatmapIdentifierNetSerializable *beatmapId, GlobalNamespace::GameplayModifiers *gameplayModifiers, float startTime) {
		for(int i = 0; i < playerData.len; ++i)
			playerData.lockedModifiers[i] = (playerData.modifiers[i].songSpeed == gameplayModifiers->get_songSpeed()) ? playerData.modifiers[i] : PlayerData::ModifiersWeCareAbout(gameplayModifiers);
	}
	public:
	static void HandleSetIsEntitledToLevel(GlobalNamespace::IConnectedPlayer *player, StringW levelId, GlobalNamespace::EntitlementsStatus entitlementStatus) {
		if(!player || levelId != selectedLevelId)
			return;
		LoadState state;
		switch(entitlementStatus) {
			case GlobalNamespace::EntitlementsStatus::NotOwned: state = LoadState_Failed; break;
			case GlobalNamespace::EntitlementsStatus::NotDownloaded: state = LoadState_Downloading; break;
			case GlobalNamespace::EntitlementsStatus::Ok: state = LoadState_Done; break;
			default: return;
		};
		playerData.cells[player->get_sortIndex()].UpdateData(LoadProgress{0, state, 0}, true);
	}
	static void HandleLoadProgress(LoadProgress packet, GlobalNamespace::IConnectedPlayer *player) {
		playerData.cells[player->get_sortIndex()].UpdateData(packet);
	}
} static handler;
std::u16string_view PacketHandler::selectedLevelId = std::u16string_view();

// public class Downloader {

static void UnityEventBase_RemoveAllListeners(UnityEngine::Events::UnityEventBase *event) {
	UnityEngine::Events::InvokableCallList *calls = event->m_Calls;
	calls->m_RuntimeCalls->Clear();
	calls->m_NeedsUpdate = true;
}

DEFINE_CLASS_CODEGEN(BeatUpClient::UI, ToggleSetting, GlobalNamespace::SwitchSettingsController,
	bool *value;
	DEFINE_CTOR(ctor, () {
		toggle = ::GetComponent<UnityEngine::UI::Toggle>(get_gameObject()->get_transform()->GetChild(1)->get_gameObject());
		UnityEventBase_RemoveAllListeners(toggle->onValueChanged);
	});
	DEFINE_OVERRIDE_METHOD(bool, GetInitValue, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::SwitchSettingsController::GetInitValue>::get(), () {
		return *value;
	});
	DEFINE_OVERRIDE_METHOD(void, ApplyValue, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::SwitchSettingsController::ApplyValue>::get(), (bool value) {
		*this->value = value;
	});
)
DEFINE_CLASS_CODEGEN(BeatUpClient::UI, ValuePickerSetting, GlobalNamespace::ListSettingsController,
	std::vector<uint8_t> options;
	float *value;
	int32_t startIdx;
	DEFINE_CTOR(ctor, () {
		stepValuePicker = ::GetComponent<GlobalNamespace::StepValuePicker>(get_gameObject()->get_transform()->GetChild(1)->get_gameObject());
	});
	DEFINE_OVERRIDE_METHOD(bool, GetInitValues, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::ListSettingsController::GetInitValues>::get(), (int32_t *idx, int32_t *numberOfElements) {
		startIdx = 0;
		*idx = 0;
		for(uint32_t i = 1; i < options.size(); ++i) {
			if(options[i] == uint8_t(*value * 4)) {
				startIdx = 1;
				*idx = i - 1;
			}
		}
		*numberOfElements = options.size() - startIdx;
		return true;
	});
	DEFINE_OVERRIDE_METHOD(void, ApplyValue, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::ListSettingsController::ApplyValue>::get(), (int32_t idx) {
		*value = options[idx + startIdx] / 4.0f;
	});
	DEFINE_OVERRIDE_METHOD(Il2CppString*, TextForValue, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::ListSettingsController::TextForValue>::get(), (int32_t idx) {
		char text[16];
		snprintf(text, sizeof(text), "%f", options[idx + startIdx] / 4.f);
		return StringW(std::string_view(text));
	});
)

DEFINE_CLASS_CODEGEN(BeatUpClient::UI, DropdownSetting, GlobalNamespace::DropdownSettingsController,
	std::vector<std::pair<std::string, std::string>> options;
	GlobalNamespace::StringSO *value;
	DEFINE_CTOR(ctor, () {
		dropdown = ::GetComponent<HMUI::SimpleTextDropdown>(get_gameObject());
	});
	DEFINE_OVERRIDE_METHOD(bool, GetInitValues, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::DropdownSettingsController::GetInitValues>::get(), (int32_t *idx, int32_t *numberOfElements) {
		*idx = 0;
		std::string ref = to_utf8(csstrtostr(value->get_value()));
		for(uint32_t i = 0; i < options.size(); ++i) {
			if(options[i].first == ref) {
				*idx = i + Config::EnableOfficialServer;
				break;
			}
		}
		*numberOfElements = options.size() + Config::EnableOfficialServer;
		return true;
	});
	DEFINE_OVERRIDE_METHOD(void, ApplyValue, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::DropdownSettingsController::ApplyValue>::get(), (int32_t idx) {
		std::string_view newValue = idx >= Config::EnableOfficialServer ? options[idx - Config::EnableOfficialServer].first : ""sv;
		dropdown->Hide(to_utf8(csstrtostr(value->get_value())) == newValue); // Animation will break if MultiplayerLevelSelectionFlowCoordinator is immediately dismissed
		value->set_value(newValue);
	});
	DEFINE_OVERRIDE_METHOD(Il2CppString*, TextForValue, il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::DropdownSettingsController::TextForValue>::get(), (int32_t idx) {
		return StringW(idx >= Config::EnableOfficialServer ? options[idx - Config::EnableOfficialServer].first : "Official Server"sv);
	});
	public:
	void SetOptions(std::vector<std::pair<std::string, std::string>> &options) {
		this->options = options;
		UnityEngine::GameObject *gameObject = get_gameObject();
		if(gameObject->get_activeSelf()) {
			GameObject_SetActive(gameObject, false);
			GameObject_SetActive(gameObject, true);
		}
	}
)

BSC_ICALL(Resources_FindObjectsOfTypeAll, "UnityEngine.Resources::FindObjectsOfTypeAll", Array<UnityEngine::Object*>*, System::Type *type);
template<typename T> static inline ArrayW<T*> FindResourcesOfType() {
	return Resources_FindObjectsOfTypeAll(csTypeOf(T*));
}

template<typename T> static std::optional<T*> FindResourceOfType() {
	ArrayW<T*> array = FindResourcesOfType<T>();
	if(array.Length() < 1)
		return std::nullopt;
	return array[0];
}

namespace BeatUpClient::UI {
	static UnityEngine::GameObject *CreateElement(UnityEngine::GameObject *template_, UnityEngine::Transform *parent, const char *name) {
		UnityEngine::GameObject *gameObject = il2cpp_utils::cast<UnityEngine::GameObject>(Object_Internal_CloneSingleWithParent(template_, parent, false));
		Object_SetName(gameObject, StringW("BeatUpClient_"sv + name));
		GameObject_SetActive(gameObject, false);
		return gameObject;
	}
	static UnityEngine::GameObject *CreateElementWithText(UnityEngine::GameObject *template_, UnityEngine::Transform *parent, const char *name, const char *headerKey) {
		UnityEngine::GameObject *gameObject = CreateElement(template_, parent, name);
		UnityEngine::RectTransform *rectTransform = il2cpp_utils::cast<UnityEngine::RectTransform>(gameObject->get_transform());
		rectTransform->set_sizeDelta(UnityEngine::Vector2(90, rectTransform->get_sizeDelta().y));
		UnityEngine::GameObject *nameText = rectTransform->Find("NameText"sv)->get_gameObject();
		GetComponent<Polyglot::LocalizedTextMeshProUGUI>(nameText)->set_Key(std::string_view(headerKey));
		return gameObject;
	}
	static UnityEngine::RectTransform *CreateToggle(UnityEngine::Transform *parent, const char *name, const char *headerKey, bool *value) {
		UnityEngine::GameObject *toggleTemplate = CRASH_UNLESS([]() {
			for(UnityEngine::UI::Toggle *toggle : FindResourcesOfType<UnityEngine::UI::Toggle>())
				if(UnityEngine::GameObject *gameObject = toggle->get_transform()->get_parent()->get_gameObject(); to_utf8(csstrtostr(gameObject->get_name())) == "Fullscreen")
					return gameObject;
			return (UnityEngine::GameObject*)NULL;
		}());
		UnityEngine::GameObject *gameObject = CreateElementWithText(toggleTemplate, parent, name, headerKey);
		Object_Destroy(GameObject_GetComponent(gameObject, csTypeOf(GlobalNamespace::BoolSettingsController*)), 0);
		ToggleSetting *toggleSetting = gameObject->AddComponent<ToggleSetting*>();
		toggleSetting->value = value;
		GameObject_SetActive(gameObject, true);
		return il2cpp_utils::cast<UnityEngine::RectTransform>(gameObject->get_transform());
	}
	static UnityEngine::RectTransform *CreateValuePicker(UnityEngine::Transform *parent, const char *name, const char *headerKey, float *value, std::vector<uint8_t> options) {
		UnityEngine::GameObject *pickerTemplate = CRASH_UNLESS([]() {
			for(GlobalNamespace::StepValuePicker *picker : FindResourcesOfType<GlobalNamespace::StepValuePicker>())
				if(UnityEngine::GameObject *gameObject = picker->get_transform()->get_parent()->get_gameObject(); to_utf8(csstrtostr(gameObject->get_name())) == "MaxNumberOfPlayers")
					return gameObject;
			return (UnityEngine::GameObject*)NULL;
		}());
		UnityEngine::GameObject *gameObject = CreateElementWithText(pickerTemplate, parent, name, headerKey);
		Object_Destroy(GameObject_GetComponent(gameObject, csTypeOf(GlobalNamespace::FormattedFloatListSettingsController*)), 0);
		ValuePickerSetting *valuePickerSetting = gameObject->AddComponent<ValuePickerSetting*>();
		valuePickerSetting->options = options;
		valuePickerSetting->value = value;
		GameObject_SetActive(gameObject, true);
		return il2cpp_utils::cast<UnityEngine::RectTransform>(gameObject->get_transform());
	}
	static UnityEngine::RectTransform *CreateSimpleDropdown(UnityEngine::Transform *parent, const char *name, GlobalNamespace::StringSO *value, std::vector<std::pair<std::string, std::string>> &options) {
		UnityEngine::GameObject *simpleDropdownTemplate = CRASH_UNLESS([]() {
			for(HMUI::SimpleTextDropdown *dropdown : FindResourcesOfType<HMUI::SimpleTextDropdown>())
				if(UnityEngine::GameObject *gameObject = dropdown->get_gameObject(); GameObject_GetComponentsInternal(gameObject, csTypeOf(UnityEngine::Component*), true, false, true, false, NULL)->get_Length() == 2)
					return gameObject;
			return (UnityEngine::GameObject*)NULL;
		}());
		UnityEngine::GameObject *gameObject = CreateElement(simpleDropdownTemplate, parent, name);
		DropdownSetting *dropdownSetting = gameObject->AddComponent<DropdownSetting*>();
		dropdownSetting->value = value;
		dropdownSetting->SetOptions(options);
		GameObject_SetActive(gameObject, true);
		return il2cpp_utils::cast<UnityEngine::RectTransform>(gameObject->get_transform());
	}
	template<class T> static UnityEngine::RectTransform *CreateTextbox(UnityEngine::Transform *parent, const char *name, int32_t index, void (T::*callback)(HMUI::InputFieldView*), T *instance, std::string_view placeholderKey = "") {
		UnityEngine::GameObject *textboxTemplate = CRASH_UNLESS(FindResourceOfType<GlobalNamespace::EnterPlayerGuestNameViewController>())->nameInputFieldView->get_gameObject();
		UnityEngine::GameObject *gameObject = CreateElement(textboxTemplate, parent, name);
		HMUI::InputFieldView *inputField = GetComponent<HMUI::InputFieldView>(gameObject);
		HMUI::InputFieldView::InputFieldChanged *onValueChanged = CRASH_UNLESS(il2cpp_utils::New<HMUI::InputFieldView::InputFieldChanged*>());
		inputField->set_onValueChanged(onValueChanged);
		onValueChanged->AddListener(MakeDelegate<UnityEngine::Events::UnityAction_1<HMUI::InputFieldView*>*>(callback, instance));
		Polyglot::LocalizedTextMeshProUGUI *localizedText = GetComponent<Polyglot::LocalizedTextMeshProUGUI>(inputField->placeholderText);
		localizedText->set_enabled(placeholderKey.length() > 0);
		localizedText->set_Key(placeholderKey);
		gameObject->get_transform()->SetSiblingIndex(index);
		GameObject_SetActive(gameObject, true);
		return il2cpp_utils::cast<UnityEngine::RectTransform>(gameObject->get_transform());
	}
	static UnityEngine::GameObject *AddKey(HMUI::UIKeyboard *keyboard, bool numpad, int32_t row, int32_t col, UnityEngine::KeyCode keyCode, Il2CppChar charCode, bool canBeUppercase = false) {
		UnityEngine::Transform *parent = keyboard->get_transform()->Find(numpad ? "Numpad"sv : "Letters"sv)->GetChild(row);
		UnityEngine::Transform *refKey = parent->GetChild(numpad ? 0 : 1);
		UnityEngine::GameObject *key = il2cpp_utils::cast<UnityEngine::GameObject>(Object_Internal_CloneSingleWithParent(refKey->get_gameObject(), parent, false));
		Il2CppChar name[] = {charCode, 0};
		key->set_name(std::u16string_view(name));
		if(col < 0)
			col += parent->get_childCount();
		key->get_transform()->SetSiblingIndex(col);
		if(numpad) {
			GetComponent<UnityEngine::UI::HorizontalLayoutGroup>(parent->get_gameObject())->set_enabled(true);
		} else {
			UnityEngine::Vector3 refPosition = refKey->get_localPosition();
			for(int i = col, count = parent->get_childCount(); i < count; ++i)
				parent->GetChild(i)->set_localPosition(UnityEngine::Vector3(refPosition.x + i * 7 - 7, refPosition.y, refPosition.z));
		}
		HMUI::UIKeyboardKey *keyboardKey = GetComponent<HMUI::UIKeyboardKey>(key);
		keyboardKey->keyCode = keyCode;
		keyboardKey->overrideText = std::u16string_view(name);
		keyboardKey->canBeUppercase = canBeUppercase;
		keyboardKey->Awake();
		keyboard->buttonBinder->AddBinding(GetComponent<HMUI::NoTransitionsButton>(key), il2cpp_utils::MakeDelegate<System::Action*>((std::function<void()>)[keyboard, charCode]() {
			if(System::Action_1<::Il2CppChar> *evt = keyboard->keyWasPressedEvent; evt)
				evt->Invoke(charCode);
		}));
		return key;
	}
	static UnityEngine::RectTransform *CreateButtonFrom(UnityEngine::GameObject *template_, UnityEngine::Transform *parent, const char *name, void (*callback)()) {
		UnityEngine::GameObject *gameObject = CreateElement(template_, parent, name);
		UnityEngine::UI::Button::ButtonClickedEvent *onClick = GetComponent<UnityEngine::UI::Button>(gameObject)->get_onClick();
		UnityEventBase_RemoveAllListeners(onClick);
		onClick->AddListener(MakeDelegate<UnityEngine::Events::UnityAction*>(callback));
		GameObject_SetActive(gameObject, true);
		return il2cpp_utils::cast<UnityEngine::RectTransform>(gameObject->get_transform());
	}
	static UnityEngine::RectTransform *CreateClone(UnityEngine::GameObject *template_, UnityEngine::Transform *parent, const char *name, int32_t index = -1) {
		UnityEngine::GameObject *gameObject = CreateElement(template_, parent, name);
		if(index >= 0)
			gameObject->get_transform()->SetSiblingIndex(index);
		GameObject_SetActive(gameObject, true);
		return il2cpp_utils::cast<UnityEngine::RectTransform>(gameObject->get_transform());
	}
}

#ifndef DISABLE_SELECTORS
class DifficultyPanel {
	static UnityEngine::GameObject *characteristicTemplate;
	static UnityEngine::GameObject *difficultyTemplate;
	GlobalNamespace::BeatmapCharacteristicSegmentedControlController *characteristicSelector = NULL;
	GlobalNamespace::BeatmapDifficultySegmentedControlController *difficultySelector = NULL;
	bool hideHints;

	static UnityEngine::GameObject *Clone(UnityEngine::GameObject *template_) {
		UnityEngine::GameObject *gameObject = il2cpp_utils::cast<UnityEngine::GameObject>(Object_Internal_CloneSingle(template_));
		UnityEngine::RectTransform *tr = il2cpp_utils::cast<UnityEngine::RectTransform>(gameObject->get_transform());
		UnityEngine::RectTransform *bg = il2cpp_utils::cast<UnityEngine::RectTransform>(tr->Find("BG"sv));
		tr->set_pivot(UnityEngine::Vector2(.5f, 1));
		bg->set_pivot(UnityEngine::Vector2(.5f, 1));
		bg->set_localPosition(UnityEngine::Vector3(0, 0, 0));
		Object_DontDestroyOnLoad(gameObject);
		return gameObject;
	}
	public:
	static void Init() {
		GlobalNamespace::StandardLevelDetailView *LevelDetail = CRASH_UNLESS(FindResourceOfType<GlobalNamespace::StandardLevelDetailView>());
		characteristicTemplate = Clone(LevelDetail->beatmapCharacteristicSegmentedControlController->get_transform()->get_parent()->get_gameObject());
		difficultyTemplate = Clone(LevelDetail->beatmapDifficultySegmentedControlController->get_transform()->get_parent()->get_gameObject());
	}
	private:
	static void ChangeBackground(UnityEngine::RectTransform *target, HMUI::ImageView *newBG) {
		HMUI::ImageView *bg = GetComponent<HMUI::ImageView>(target->Find("BG")->get_gameObject());
		bg->skew = newBG->skew, bg->set_color(newBG->get_color()), bg->set_color0(newBG->get_color0()), bg->set_color1(newBG->get_color1()), bg->flipGradientColors = newBG->flipGradientColors;
	}
	public:
	DifficultyPanel() {}
	void Configure(UnityEngine::Transform *parent, int32_t index, float width, HMUI::ImageView *background = NULL, bool hideHints = false) {
		UnityEngine::RectTransform *beatmapCharacteristic = BeatUpClient::UI::CreateClone(characteristicTemplate, parent, "BeatmapCharacteristic", index);
		UnityEngine::RectTransform *beatmapDifficulty = BeatUpClient::UI::CreateClone(difficultyTemplate, parent, "BeatmapDifficulty", index + 1);
		if(background) {
			ChangeBackground(beatmapCharacteristic, background);
			ChangeBackground(beatmapDifficulty, background);
		}
		beatmapCharacteristic->set_sizeDelta(UnityEngine::Vector2(width, beatmapCharacteristic->get_sizeDelta().y));
		beatmapDifficulty->set_sizeDelta(UnityEngine::Vector2(width, beatmapDifficulty->get_sizeDelta().y));
		characteristicSelector = il2cpp_utils::cast<GlobalNamespace::BeatmapCharacteristicSegmentedControlController>(GameObject_GetComponentInChildren(beatmapCharacteristic->get_gameObject(), csTypeOf(GlobalNamespace::BeatmapCharacteristicSegmentedControlController*), false));
		difficultySelector = il2cpp_utils::cast<GlobalNamespace::BeatmapDifficultySegmentedControlController>(GameObject_GetComponentInChildren(beatmapDifficulty->get_gameObject(), csTypeOf(GlobalNamespace::BeatmapDifficultySegmentedControlController*), false));
		characteristicSelector->segmentedControl->container = CRASH_UNLESS(il2cpp_utils::New<Zenject::DiContainer*>());
		characteristicSelector->segmentedControl->container->Bind(ArrayW<System::Type*>({(System::Type*)csTypeOf(HMUI::HoverHintController*)}))->FromInstance(CRASH_UNLESS(FindResourceOfType<HMUI::HoverHintController>()))->AsSingle();
		difficultySelector->difficultySegmentedControl->container = CRASH_UNLESS(il2cpp_utils::New<Zenject::DiContainer*>());
		this->hideHints = hideHints;
	}
	bool Clear() {
		GameObject_SetActive(characteristicSelector->get_transform()->get_parent()->get_gameObject(), false);
		GameObject_SetActive(difficultySelector->get_transform()->get_parent()->get_gameObject(), false);
		return false;
	}
	template<class L> bool Update(GlobalNamespace::PreviewDifficultyBeatmap *beatmapLevel, L &&onChange) {
		return Update(beatmapLevel, std::function(onChange));
	}
	bool Update(GlobalNamespace::PreviewDifficultyBeatmap *beatmapLevel, std::function<void(GlobalNamespace::PreviewDifficultyBeatmap)> onChange) {
		characteristicSelector->didSelectBeatmapCharacteristicEvent = MakeDelegate<System::Action_2<GlobalNamespace::BeatmapCharacteristicSegmentedControlController*, GlobalNamespace::BeatmapCharacteristicSO*>*>((void (*)(GlobalNamespace::BeatmapCharacteristicSegmentedControlController*, GlobalNamespace::BeatmapCharacteristicSO*))[](GlobalNamespace::BeatmapCharacteristicSegmentedControlController*, GlobalNamespace::BeatmapCharacteristicSO*){});
		difficultySelector->didSelectDifficultyEvent = MakeDelegate<System::Action_2<GlobalNamespace::BeatmapDifficultySegmentedControlController*, GlobalNamespace::BeatmapDifficulty>*>((void (*)(GlobalNamespace::BeatmapDifficultySegmentedControlController*, GlobalNamespace::BeatmapDifficulty))[](GlobalNamespace::BeatmapDifficultySegmentedControlController*, GlobalNamespace::BeatmapDifficulty){});
		if(!beatmapLevel)
			return Clear();
		System::Collections::Generic::IReadOnlyList_1<GlobalNamespace::PreviewDifficultyBeatmapSet*> *previewDifficultyBeatmapSets = beatmapLevel->get_beatmapLevel()->get_previewDifficultyBeatmapSets();
		if(!previewDifficultyBeatmapSets)
			return Clear();
		uint32_t beatmapCharacteristics_len = ((System::Collections::Generic::IReadOnlyCollection_1<GlobalNamespace::PreviewDifficultyBeatmapSet*>*)previewDifficultyBeatmapSets)->get_Count();
		GlobalNamespace::CustomDifficultyBeatmapSet beatmapCharacteristics[beatmapCharacteristics_len];
		ArrayW<GlobalNamespace::IDifficultyBeatmapSet*> beatmapCharacteristic_ptrs = ArrayW<GlobalNamespace::IDifficultyBeatmapSet*>(beatmapCharacteristics_len);
		for(uint32_t i = 0; i < beatmapCharacteristics_len; ++i) {
			GlobalNamespace::PreviewDifficultyBeatmapSet *set = previewDifficultyBeatmapSets->get_Item(i);
			GlobalNamespace::BeatmapCharacteristicSO *beatmapCharacteristic = set->get_beatmapCharacteristic();
			if(beatmapCharacteristic == beatmapLevel->get_beatmapCharacteristic()) {
				ArrayW<GlobalNamespace::BeatmapDifficulty> beatmapDifficulties = set->get_beatmapDifficulties();
				uint32_t difficultyBeatmaps_len = beatmapDifficulties.Length();
				GlobalNamespace::CustomDifficultyBeatmap difficultyBeatmaps[difficultyBeatmaps_len];
				ArrayW<GlobalNamespace::IDifficultyBeatmap*> difficultyBeatmap_ptrs = ArrayW<GlobalNamespace::IDifficultyBeatmap*>(difficultyBeatmaps_len);
				for(uint32_t i = 0; i < difficultyBeatmaps_len; ++i) {
					difficultyBeatmaps[i] = GlobalNamespace::CustomDifficultyBeatmap{
						.difficulty = beatmapDifficulties[i],
					};
					difficultyBeatmap_ptrs[i] = (GlobalNamespace::IDifficultyBeatmap*)&difficultyBeatmaps[i];
				}
				difficultySelector->SetData((System::Collections::Generic::IReadOnlyList_1<GlobalNamespace::IDifficultyBeatmap*>*)difficultyBeatmap_ptrs.convert(), beatmapLevel->get_beatmapDifficulty());
			}
			beatmapCharacteristics[i] = GlobalNamespace::CustomDifficultyBeatmapSet{
				.beatmapCharacteristic = beatmapCharacteristic,
			};
			beatmapCharacteristic_ptrs[i] = (GlobalNamespace::IDifficultyBeatmapSet*)&beatmapCharacteristics[i];
		}
		characteristicSelector->SetData((System::Collections::Generic::IReadOnlyList_1<GlobalNamespace::IDifficultyBeatmapSet*>*)beatmapCharacteristic_ptrs.convert(), beatmapLevel->get_beatmapCharacteristic());
		/*if(hideHints) // TODO: make this work
			foreach(HMUI.HoverHint hint in characteristicSelector.GetComponentsInChildren<HMUI.HoverHint>())
				hint.enabled = false;*/
		characteristicSelector->add_didSelectBeatmapCharacteristicEvent(MakeDelegate<System::Action_2<GlobalNamespace::BeatmapCharacteristicSegmentedControlController*, GlobalNamespace::BeatmapCharacteristicSO*>*>([=](GlobalNamespace::BeatmapCharacteristicSegmentedControlController *controller, GlobalNamespace::BeatmapCharacteristicSO *characteristic) {
				GlobalNamespace::PreviewDifficultyBeatmapSet *set;
				for(uint32_t i = 0; i < beatmapCharacteristics_len; ++i) {
					set = previewDifficultyBeatmapSets->get_Item(i);
					if(set->get_beatmapCharacteristic() == characteristic)
						break;
				}
				ArrayW<GlobalNamespace::BeatmapDifficulty> beatmapDifficulties = set->get_beatmapDifficulties();
				GlobalNamespace::BeatmapDifficulty closestDifficulty = beatmapDifficulties[0];
				for(BeatmapDifficulty difficulty : beatmapDifficulties) {
					if(beatmapLevel->get_beatmapDifficulty() < difficulty)
						break;
					closestDifficulty = difficulty;
				}
				onChange(GlobalNamespace::PreviewDifficultyBeatmap{
					.beatmapLevel = beatmapLevel->get_beatmapLevel(),
					.beatmapCharacteristic = characteristic,
					.beatmapDifficulty = closestDifficulty,
				});
			}
		));
		difficultySelector->add_didSelectDifficultyEvent(MakeDelegate<System::Action_2<GlobalNamespace::BeatmapDifficultySegmentedControlController*, GlobalNamespace::BeatmapDifficulty>*>([=](GlobalNamespace::BeatmapDifficultySegmentedControlController *controller, GlobalNamespace::BeatmapDifficulty difficulty) {
			onChange(GlobalNamespace::PreviewDifficultyBeatmap{
				.beatmapLevel = beatmapLevel->get_beatmapLevel(),
				.beatmapCharacteristic = beatmapLevel->get_beatmapCharacteristic(),
				.beatmapDifficulty = difficulty,
			});
		}));
		GameObject_SetActive(characteristicSelector->get_transform()->get_parent()->get_gameObject(), true);
		GameObject_SetActive(difficultySelector->get_transform()->get_parent()->get_gameObject(), true);
		return false;
	}
} static lobbyDifficultyPanel, gameplayDifficultyPanel;
UnityEngine::GameObject *DifficultyPanel::characteristicTemplate = NULL;
UnityEngine::GameObject *DifficultyPanel::difficultyTemplate = NULL;
#endif

static GlobalNamespace::CustomNetworkConfig *customNetworkConfig = NULL;
BSC_MAKE_HOOK_MATCH(GlobalNamespace::ClientCertificateValidator::ValidateCertificateChainInternal, void, GlobalNamespace::ClientCertificateValidator *self, ::GlobalNamespace::DnsEndPoint* endPoint, ::System::Security::Cryptography::X509Certificates::X509Certificate2* certificate, ::ArrayW<::ArrayW<uint8_t>> certificateChain) {
	if(!customNetworkConfig) {
		logger->info("running ValidateCertificateChainInternal()");
		base(self, endPoint, certificate, certificateChain);
	} else {
		logger->info("bypassing ValidateCertificateChainInternal()");
	}
}

static GlobalNamespace::StringSO *customServerHostName = NULL;
static bool prev_useCustomServerEnvironment = false;
BSC_MAKE_HOOK_MATCH(GlobalNamespace::MainSettingsModelSO::Load, void, GlobalNamespace::MainSettingsModelSO *self, bool forced) {
	base(self, forced);
	customServerHostName = self->customServerHostName;
	std::string hostname = to_utf8(csstrtostr(customServerHostName->get_value()));
	if(hostname.length()) {
		if(std::accumulate(Config::Servers.begin(), Config::Servers.end(), 0u, [hostname](uint32_t total, std::pair<std::string, std::string> &e) {return total + (e.first == hostname);}) == 0)
			Config::Servers.push_back(std::pair(hostname, ""));
	} else if(!Config::EnableOfficialServer) {
		customServerHostName->set_value(Config::Servers.size() ? Config::Servers[0].first : Config::DefaultServer.first);
	}
	prev_useCustomServerEnvironment = self->useCustomServerEnvironment->get_value();
	self->useCustomServerEnvironment->set_value(true);
	logger->info("GlobalNamespace::MainSettingsModelSO::Load()");
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MainSettingsModelSO::Save, void, GlobalNamespace::MainSettingsModelSO *self) {
	logger->info("GlobalNamespace::MainSettingsModelSO::Save()");
	self->useCustomServerEnvironment->set_value(prev_useCustomServerEnvironment);
	base(self);
	self->useCustomServerEnvironment->set_value(true);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::PlatformAuthenticationTokenProvider::GetAuthenticationToken, System::Threading::Tasks::Task_1<GlobalNamespace::AuthenticationToken>*, GlobalNamespace::PlatformAuthenticationTokenProvider* self) {
	if(!customNetworkConfig)
		return base(self);
	return System::Threading::Tasks::Task_1<GlobalNamespace::AuthenticationToken>::New_ctor(
		GlobalNamespace::AuthenticationToken(self->platform, self->userId, self->userName, Array<uint8_t>::NewLength(0)));
}

static UnityEngine::GameObject *prevLoading = NULL;
static void ShowLoading(UnityEngine::GameObject *loading) {
	if(prevLoading)
		Object_Destroy(prevLoading, 0);
	prevLoading = loading;
	if(loading) {
		Object_SetName(loading, StringW("BeatUpClient_LoadingControl"sv));
		loading->get_transform()->set_localPosition(UnityEngine::Vector3(0, 12, 0));
		GetComponent<GlobalNamespace::LoadingControl>(loading)->ShowLoading(Polyglot::Localization::Get("LABEL_CHECKING_SERVER_STATUS"sv));
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
		ShowLoading(il2cpp_utils::cast<UnityEngine::GameObject>(Object_Internal_CloneSingleWithParent(originalView->loadingControl->get_gameObject(), mainViewController->get_transform(), false)));
	}
	base(self, mainViewController, leftScreenViewController, rightScreenViewController, bottomScreenViewController, topScreenViewController);
}

// This callback triggers twice if `_multiplayerStatusModel.GetMultiplayerStatusAsync()` was cancelled by pressing the back button
BSC_MAKE_HOOK_MATCH(GlobalNamespace::MainFlowCoordinator::HandleMultiplayerModeSelectionFlowCoordinatorDidFinish, void, GlobalNamespace::MainFlowCoordinator *self, ::GlobalNamespace::MultiplayerModeSelectionFlowCoordinator* multiplayerModeSelectionFlowCoordinator) {
	if(multiplayerModeSelectionFlowCoordinator->parentFlowCoordinator)
		base(self, multiplayerModeSelectionFlowCoordinator);
}

// The UI deletes itself at the end of `MultiplayerModeSelectionFlowCoordinator.TryShowModeSelection()` without this
// TODO: Figure out how to use `MAKE_HOOK_MATCH` with overloaded functions
// BSC_MAKE_HOOK_MATCH(HMUI::FlowCoordinator::ReplaceTopViewController, void, HMUI::FlowCoordinator *self, ::HMUI::ViewController* viewController, ::System::Action* finishedCallback, ::HMUI::ViewController::AnimationType animationType, ::HMUI::ViewController::AnimationDirection animationDirection) {
BSC_MAKE_HOOK_FIND_CLASS_INSTANCE("HMUI", "FlowCoordinator", "ReplaceTopViewController", void, HMUI::FlowCoordinator *self, ::HMUI::ViewController* viewController, ::System::Action* finishedCallback, ::HMUI::ViewController::AnimationType animationType, ::HMUI::ViewController::AnimationDirection animationDirection) {
	if(il2cpp_utils::try_cast<GlobalNamespace::MultiplayerModeSelectionViewController>(viewController).value_or(nullptr))
		self->TopViewControllerWillChange(viewController, viewController, animationType);
	else
		base(self, viewController, finishedCallback, animationType, animationDirection);
}

static std::string GetMaintenanceMessage(GlobalNamespace::MultiplayerUnavailableReason reason, System::Nullable_1<int64_t> maintenanceWindowEndTime) {
	if(reason == GlobalNamespace::MultiplayerUnavailableReason::MaintenanceMode)
		return Polyglot::Localization::GetFormat(GlobalNamespace::MultiplayerUnavailableReasonMethods::LocalizedKey(reason), ArrayW<Il2CppObject*>({(Il2CppObject*)(GlobalNamespace::TimeExtensions::AsUnixTime(maintenanceWindowEndTime.GetValueOrDefault()) - System::DateTime::get_UtcNow()).ToString("h':'mm")}));
	return Polyglot::Localization::Get(GlobalNamespace::MultiplayerUnavailableReasonMethods::LocalizedKey(reason)) + " (" + GlobalNamespace::MultiplayerUnavailableReasonMethods::ErrorCode(reason) + ")";
}

static GlobalNamespace::MainFlowCoordinator *mainFlowCoordinator = NULL;
BSC_MAKE_HOOK_MATCH(GlobalNamespace::MultiplayerModeSelectionFlowCoordinator::PresentMasterServerUnavailableErrorDialog, void, GlobalNamespace::MultiplayerModeSelectionFlowCoordinator *self, ::GlobalNamespace::MultiplayerUnavailableReason reason, ::System::Exception* exception, ::System::Nullable_1<int64_t> maintenanceWindowEndTime, ::StringW remoteLocalizedMessage) {
	if(mainFlowCoordinator->get_childFlowCoordinator() != self)
		return;
	ShowLoading(NULL);
	TMPro::TextMeshProUGUI *message = self->multiplayerModeSelectionViewController->maintenanceMessageText;
	message->set_text(remoteLocalizedMessage ? remoteLocalizedMessage : StringW(GetMaintenanceMessage(reason, maintenanceWindowEndTime)));
	message->set_richText(true);
	message->get_transform()->set_localPosition(UnityEngine::Vector3(0, 15, 0));
	GameObject_SetActive(message->get_gameObject(), true);
	self->SetTitle(Polyglot::Localization::Get("LABEL_CONNECTION_ERROR"sv), HMUI::ViewController::AnimationType::In);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MultiplayerModeSelectionViewController::SetData, void, GlobalNamespace::MultiplayerModeSelectionViewController *self, ::GlobalNamespace::MultiplayerStatusData* multiplayerStatusData) {
	ShowLoading(NULL);
	self->maintenanceMessageText->set_richText(false);
	self->maintenanceMessageText->get_transform()->set_localPosition(UnityEngine::Vector3(0, -5, 0));
	if(UnityEngine::Transform *buttons = self->get_transform()->Find("Buttons"); buttons)
		GameObject_SetActive(buttons->get_gameObject(), true);
	base(self, multiplayerStatusData);
}

// [Patch(PatchType.Prefix, typeof(LobbySetupViewController), nameof(LobbySetupViewController.SetPlayersMissingLevelText))]

static bool enableCustomLevels = false;
void HandleConnectToServerSuccess(bool notGameLift, GlobalNamespace::BeatmapLevelSelectionMask selectionMask, GlobalNamespace::GameplayServerConfiguration configuration) {
	logger->info("HandleConnectToServerSuccess()");
	enableCustomLevels = notGameLift && selectionMask.songPacks.Contains(StringW("custom_levelpack_CustomLevels"sv));
	playerData.Init(configuration.maxPlayerCount);
	#ifndef DISABLE_SELECTORS
	lobbyDifficultyPanel.Clear();
	#endif
	connectInfo = DefaultConnectInfo();
	clientActive = false;
	GameObject_SetActive(infoText, false);
	logger->info("HandleConnectToServerSuccess() end");
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MasterServerConnectionManager::HandleConnectToServerSuccess, void, GlobalNamespace::MasterServerConnectionManager *self, ::StringW remoteUserId, ::StringW remoteUserName, ::System::Net::IPEndPoint* remoteEndPoint, ::StringW secret, ::StringW code, ::GlobalNamespace::BeatmapLevelSelectionMask selectionMask, ::GlobalNamespace::GameplayServerConfiguration configuration, ::ArrayW<uint8_t> preMasterSecret, ::ArrayW<uint8_t> myRandom, ::ArrayW<uint8_t> remoteRandom, bool isConnectionOwner, bool isDedicatedServer, ::StringW managerId) {
	HandleConnectToServerSuccess(true, selectionMask, configuration);
	base(self, remoteUserId, remoteUserName, remoteEndPoint, secret, code, selectionMask, configuration, preMasterSecret, myRandom, remoteRandom, isConnectionOwner, isDedicatedServer, managerId);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::GameLiftConnectionManager::HandleConnectToServerSuccess, void, GlobalNamespace::GameLiftConnectionManager *self, ::StringW playerSessionId, ::System::Net::IPEndPoint* remoteEndPoint, ::StringW gameSessionId, ::StringW secret, ::StringW code, ::GlobalNamespace::BeatmapLevelSelectionMask selectionMask, ::GlobalNamespace::GameplayServerConfiguration configuration) {
	HandleConnectToServerSuccess(false, selectionMask, configuration);
	base(self, playerSessionId, remoteEndPoint, gameSessionId, secret, code, selectionMask, configuration);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::LevelSelectionNavigationController::Setup, void, GlobalNamespace::LevelSelectionNavigationController *self, ::GlobalNamespace::SongPackMask songPackMask, ::GlobalNamespace::BeatmapDifficultyMask allowedBeatmapDifficultyMask, ::ArrayW<::GlobalNamespace::BeatmapCharacteristicSO*> notAllowedCharacteristics, bool hidePacksIfOneOrNone, bool hidePracticeButton, ::StringW actionButtonText, ::GlobalNamespace::IBeatmapLevelPack* levelPackToBeSelectedAfterPresent, ::GlobalNamespace::SelectLevelCategoryViewController::LevelCategory startLevelCategory, ::GlobalNamespace::IPreviewBeatmapLevel* beatmapLevelToBeSelectedAfterPresent, bool _enableCustomLevels) {
	base(self, songPackMask, allowedBeatmapDifficultyMask, notAllowedCharacteristics, hidePacksIfOneOrNone, hidePracticeButton, actionButtonText, levelPackToBeSelectedAfterPresent, startLevelCategory, beatmapLevelToBeSelectedAfterPresent, _enableCustomLevels || enableCustomLevels);
}

#ifndef DISABLE_360
BSC_MAKE_HOOK_MATCH(GlobalNamespace::MultiplayerLevelSelectionFlowCoordinator::Setup, void, GlobalNamespace::MultiplayerLevelSelectionFlowCoordinator *self, ::GlobalNamespace::LevelSelectionFlowCoordinator::State* state, ::GlobalNamespace::SongPackMask songPackMask, ::GlobalNamespace::BeatmapDifficultyMask allowedBeatmapDifficultyMask, ::StringW actionText, ::StringW titleText) {
	if(self->notAllowedCharacteristics->Length())
		System::Array::Resize(ByRef(self->notAllowedCharacteristics), 0);
	base(self, state, songPackMask, allowedBeatmapDifficultyMask, actionText, titleText);
}
#endif

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MultiplayerSessionManager::HandlePlayerOrderChanged, void, GlobalNamespace::MultiplayerSessionManager *self, ::GlobalNamespace::IConnectedPlayer* player) {
	playerData.Reset(player->get_sortIndex());
	base(self, player);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::BasicConnectionRequestHandler::GetConnectionMessage, void, GlobalNamespace::BasicConnectionRequestHandler *self, ::LiteNetLib::Utils::NetDataWriter* writer, ::StringW userId, ::StringW userName, bool isConnectionOwner) {
	logger->info("GlobalNamespace::BasicConnectionRequestHandler::GetConnectionMessage()");
	base(self, writer, userId, userName, isConnectionOwner);
	uint8_t data[65536], *data_end = data;
	struct BeatUpConnectHeaderFull info = {
		.name = String_from("BeatUpClient beta0"),
		.base = {
			.protocolId = 1,
			.base = {
				.windowSize = LiteNetLib::NetConstants::DefaultWindowSize,
				.countdownDuration = uint8_t(Config::CountdownDuration * 4),
				.directDownloads = Config::DirectDownloads,
				.skipResults = Config::SkipResults,
				.perPlayerDifficulty = Config::PerPlayerDifficulty,
				.perPlayerModifiers = Config::PerPlayerModifiers,
			},
		},
	};
	struct ModConnectHeader header = {
		.length = (uint32_t)_pkt_try_write((PacketWriteFunc)_pkt_BeatUpConnectHeaderFull_write, &info, &data_end, endof(data), packetVersion),
		.name = info.name,
	};
	size_t len = _pkt_try_write((PacketWriteFunc)_pkt_ModConnectHeader_write, &header, &data_end, endof(data), packetVersion);
	len += _pkt_try_write((PacketWriteFunc)_pkt_BeatUpConnectHeader_write, &info.base, &data_end, endof(data), packetVersion);
	if(writer->autoResize)
		writer->ResizeIfNeed(writer->position + len);
	if(writer->position + len <= writer->data.Length()) {
		memcpy(&writer->data[writer->position], data, len);
		writer->position += len;
	}
	logger->info("GlobalNamespace::BasicConnectionRequestHandler::GetConnectionMessage() end");
}

BSC_MAKE_HOOK_MATCH(LiteNetLib::NetConnectAcceptPacket::FromData, ::LiteNetLib::NetConnectAcceptPacket*, ::LiteNetLib::NetPacket* packet) {
	if(packet->Size == LiteNetLib::NetConnectAcceptPacket::Size + BeatUpConnectInfo_Size) {
		packet->Size = LiteNetLib::NetConnectAcceptPacket::Size;
		BeatUpConnectInfo info;
		const uint8_t *data = &packet->RawData[packet->Size];
		_pkt_try_read((PacketReadFunc)_pkt_BeatUpConnectInfo_read, &info, &data, &data[BeatUpConnectInfo_Size], packetVersion);
		if(info.windowSize >= 32 && info.windowSize <= 512) {
			info.directDownloads = info.directDownloads && Config::DirectDownloads;
			connectInfo = info;
			clientActive = true;
			GameObject_SetActive(infoText, true);
			// logger->info("Overriding window size - %u", info.windowSize);
			logger->info("Window size - %u", info.windowSize);
		}
	}
	if(UnityEngine::Transform *tr = CRASH_UNLESS(FindResourceOfType<GlobalNamespace::GameServerPlayersTableView>())->get_transform()->Find("ServerPlayersTableHeader/Labels/SuggestedModifiers"sv); tr)
		if(Polyglot::LocalizedTextMeshProUGUI *SuggestedModifiers = TryGetComponent<Polyglot::LocalizedTextMeshProUGUI>(tr->get_gameObject()); SuggestedModifiers)
			SuggestedModifiers->set_Key((clientActive && !connectInfo.perPlayerModifiers) ? "BEATUP_VOTE_MODIFIERS"sv : "SUGGESTED_MODIFIERS"sv);
	return base(packet);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::LobbyPlayersDataModel::HandleMenuRpcManagerGetRecommendedBeatmap, void, GlobalNamespace::LobbyPlayersDataModel *self, ::StringW userId) {
	handler.Send({
		.type = BeatUpMessageType_RecommendPreview,
		.recommendPreview = playerData.previews[handler.multiplayerSessionManager->get_localPlayer()->get_sortIndex()],
	});
	base(self, userId);
}
BSC_MAKE_HOOK_MATCH(GlobalNamespace::LobbyPlayersDataModel::SetLocalPlayerBeatmapLevel, void, GlobalNamespace::LobbyPlayersDataModel *self, ::GlobalNamespace::PreviewDifficultyBeatmap* beatmapLevel) {
	if(beatmapLevel) {
		int32_t localIndex = handler.multiplayerSessionManager->get_localPlayer()->get_sortIndex();
		RecommendPreview *preview = &playerData.previews[localIndex];
		std::string levelId = to_utf8(csstrtostr(beatmapLevel->get_beatmapLevel()->get_levelID()));
		if(!String_eq_view(&preview->base.levelId, levelId)) {
			preview = playerData.ResolvePreview(levelId);
			if(preview)
				playerData.previews[localIndex] = *preview;
			else if(GlobalNamespace::CustomPreviewBeatmapLevel *previewBeatmapLevel = il2cpp_utils::try_cast<GlobalNamespace::CustomPreviewBeatmapLevel>(beatmapLevel->get_beatmapLevel()).value_or(nullptr); previewBeatmapLevel)
				preview = playerData.SetPreviewFromLocal(localIndex, previewBeatmapLevel);
		}
		if(preview) {
			handler.Send({
				.type = BeatUpMessageType_RecommendPreview,
				.recommendPreview = *preview,
			});
		}
	}
	#ifndef DISABLE_SELECTORS
	lobbyDifficultyPanel.Update(beatmapLevel, [=](GlobalNamespace::PreviewDifficultyBeatmap beatmapLevel) {
		body(self, CRASH_UNLESS(il2cpp_utils::New<GlobalNamespace::PreviewDifficultyBeatmap*>(beatmapLevel.beatmapLevel, beatmapLevel.beatmapCharacteristic, beatmapLevel.beatmapDifficulty)));
	});
	#endif
	base(self, beatmapLevel);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::LobbyPlayersDataModel::ClearLocalPlayerBeatmapLevel, void, GlobalNamespace::LobbyPlayersDataModel *self) {
	#ifndef DISABLE_SELECTORS
	lobbyDifficultyPanel.Clear();
	#endif
	base(self);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::BeatmapLevelsModel::GetLevelPreviewForLevelId, ::GlobalNamespace::IPreviewBeatmapLevel*, GlobalNamespace::BeatmapLevelsModel *self, ::StringW levelId) {
	if(GlobalNamespace::IPreviewBeatmapLevel *preview = base(self, levelId); preview)
		return preview;
	std::string levelId_str = to_utf8(csstrtostr(levelId));
	BeatUpClient::NetworkPreviewBeatmapLevel *playerPreview = CRASH_UNLESS(BeatUpClient::NetworkPreviewBeatmapLevel::New_ctor());
	playerPreview->Init(playerData.ResolvePreview(levelId_str), levelId_str);
	return (GlobalNamespace::IPreviewBeatmapLevel*)playerPreview;
}

static std::string uploadLevel = "";
static uint8_t uploadHash[32] = {};
static uint64_t uploadData_len = 0;

BSC_MAKE_HOOK_MATCH(GlobalNamespace::NetworkPlayerEntitlementChecker::GetEntitlementStatus, ::System::Threading::Tasks::Task_1<::GlobalNamespace::EntitlementsStatus>*, GlobalNamespace::NetworkPlayerEntitlementChecker *self, ::StringW levelId) {
	return base(self, levelId);
}
// [Patch(PatchType.Postfix, typeof(NetworkPlayerEntitlementChecker), "GetEntitlementStatus")]

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MenuRpcManager::SetIsEntitledToLevel, void, GlobalNamespace::MenuRpcManager *self, ::StringW levelId, ::GlobalNamespace::EntitlementsStatus entitlementStatus) {
	std::string levelId_str = to_utf8(csstrtostr(levelId));
	logger->info("MenuRpcManager_SetIsEntitledToLevel(levelId=\"%s\", entitlementStatus=%d)", levelId_str.c_str(), entitlementStatus.value);
	RecommendPreview *preview = playerData.ResolvePreview(levelId_str);
	if(PacketHandler::MissingRequirements(preview, entitlementStatus == GlobalNamespace::EntitlementsStatus::Unknown)) {
		entitlementStatus = GlobalNamespace::EntitlementsStatus::NotOwned;
	} else if(entitlementStatus == GlobalNamespace::EntitlementsStatus::Ok && uploadLevel == levelId_str) {
		logger->info("    Announcing share for `%s`", levelId_str.c_str());
		handler.Send({
			.type = BeatUpMessageType_SetCanShareBeatmap,
			.setCanShareBeatmap = new_SetCanShareBeatmap(levelId_str, uploadHash, uploadData_len),
		});
	}
	logger->info("    entitlementStatus={entitlementStatus}");
	PacketHandler::HandleSetIsEntitledToLevel(handler.multiplayerSessionManager->get_localPlayer(), levelId, entitlementStatus);
	base(self, levelId, entitlementStatus);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MultiplayerLevelLoader::LoadLevel, void, GlobalNamespace::MultiplayerLevelLoader *self, ::GlobalNamespace::ILevelGameplaySetupData* gameplaySetupData, float initialStartTime) {
	base(self, gameplaySetupData, initialStartTime);
}
// [Patch(PatchType.Prefix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.LoadLevel))]
// [Patch(PatchType.Postfix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.LoadLevel))]

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MultiplayerLevelLoader::Tick, void, GlobalNamespace::MultiplayerLevelLoader *self) {
	base(self);
}
// [Patch(PatchType.Prefix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.Tick))]
// [Patch(PatchType.Postfix, typeof(MultiplayerLevelLoader), nameof(MultiplayerLevelLoader.Tick))]

// [Patch(PatchType.Prefix, typeof(ConnectedPlayerManager), "Send", typeof(LiteNetLib.Utils.INetSerializable))]

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MultiplayerOutroAnimationController::AnimateOutro, void, GlobalNamespace::MultiplayerOutroAnimationController *self, ::GlobalNamespace::MultiplayerResultsData* multiplayerResultsData, ::System::Action* onCompleted) {
	if(connectInfo.skipResults)
		onCompleted->Invoke();
	else
		base(self, multiplayerResultsData, onCompleted);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::GameServerPlayersTableView::SetData, void, GlobalNamespace::GameServerPlayersTableView *self, ::System::Collections::Generic::List_1<::GlobalNamespace::IConnectedPlayer*>* sortedPlayers, ::GlobalNamespace::ILobbyPlayersDataModel* lobbyPlayersDataModel, bool hasKickPermissions, bool allowSelection, bool showSongSelection, bool showModifierSelection, bool clearSelection) {
	base(self, sortedPlayers, lobbyPlayersDataModel, hasKickPermissions, allowSelection, showSongSelection, showModifierSelection, clearSelection);
}
// [Patch(PatchType.Postfix, typeof(GameServerPlayersTableView), nameof(GameServerPlayersTableView.SetData))]

BSC_MAKE_HOOK_MATCH(GlobalNamespace::GameServerPlayerTableCell::Awake, void, GlobalNamespace::GameServerPlayerTableCell *self) {
	Object_SetName(Object_Internal_CloneSingleWithParent(self->localPlayerBackgroundImage->get_gameObject(), self->localPlayerBackgroundImage->get_transform(), false), StringW("BeatUpClient_Progress"sv));
	base(self);
}

BSC_MAKE_HOOK_MATCH(GlobalNamespace::ConnectedPlayerManager::HandleServerPlayerConnected, void, GlobalNamespace::ConnectedPlayerManager *self, ::GlobalNamespace::ConnectedPlayerManager::PlayerConnectedPacket* packet, ::GlobalNamespace::IConnectedPlayer* iPlayer) {
	base(self, packet, iPlayer);
}
// [Patch(PatchType.Prefix, typeof(ConnectedPlayerManager), "HandleServerPlayerConnected")]

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MultiplayerLobbyAvatarManager::AddPlayer, void, GlobalNamespace::MultiplayerLobbyAvatarManager *self, ::GlobalNamespace::IConnectedPlayer* connectedPlayer) {
	base(self, connectedPlayer);
}
// [Patch(PatchType.Postfix, typeof(MultiplayerLobbyAvatarManager), nameof(MultiplayerLobbyAvatarManager.AddPlayer))]

BSC_MAKE_HOOK_MATCH(GlobalNamespace::MultiplayerLocalActivePlayerInGameMenuViewController::Start, void, GlobalNamespace::MultiplayerLocalActivePlayerInGameMenuViewController *self) {
	base(self);
}
// [Patch(PatchType.Postfix, typeof(MultiplayerLocalActivePlayerInGameMenuViewController), nameof(MultiplayerLocalActivePlayerInGameMenuViewController.Start))]

static GlobalNamespace::MainSystemInit *mainSystemInit = NULL;
static GlobalNamespace::MultiplayerStatusModel *multiplayerStatusModel = NULL;
static GlobalNamespace::QuickPlaySetupModel *quickPlaySetupModel = NULL;
BSC_MAKE_HOOK_MATCH(GlobalNamespace::MainSystemInit::InstallBindings, void, GlobalNamespace::MainSystemInit *self, Zenject::DiContainer *container) {
	base(self, container);
	mainSystemInit = self;
	handler.Init(container);
	multiplayerStatusModel = il2cpp_utils::try_cast<GlobalNamespace::MultiplayerStatusModel>(container->TryResolve(csTypeOf(GlobalNamespace::IMultiplayerStatusModel*))).value_or(nullptr);
	quickPlaySetupModel = il2cpp_utils::try_cast<GlobalNamespace::QuickPlaySetupModel>(container->TryResolve(csTypeOf(GlobalNamespace::IQuickPlaySetupModel*))).value_or(nullptr);
	customNetworkConfig = il2cpp_utils::try_cast<GlobalNamespace::CustomNetworkConfig>(container->TryResolve(csTypeOf(GlobalNamespace::INetworkConfig*))).value_or(nullptr);
	logger->info("GlobalNamespace::MainSystemInit::InstallBindings");
}

BSC_MAKE_HOOK_FIND_CLASS_INSTANCE("LiteNetLib", "ReliableChannel", ".ctor", void, LiteNetLib::ReliableChannel *self, ::LiteNetLib::NetPeer* peer, bool ordered, uint8_t id) {
	base(self, peer, ordered, id);
	if(self->windowSize == connectInfo.windowSize)
		return;
	self->windowSize = connectInfo.windowSize;
	System::Array::Resize(ByRef(self->pendingPackets), connectInfo.windowSize);
	for(il2cpp_array_size_t i = 1, len = self->pendingPackets.Length(); i < len; ++i)
		self->pendingPackets[i] = self->pendingPackets[0];
	if(self->receivedPackets)
		System::Array::Resize(ByRef(self->receivedPackets), connectInfo.windowSize);
	if(self->earlyReceived)
		System::Array::Resize(ByRef(self->earlyReceived), connectInfo.windowSize);
	il2cpp_utils::RunMethod(self->outgoingAcks, ".ctor", LiteNetLib::PacketProperty(LiteNetLib::PacketProperty::Ack), int((connectInfo.windowSize - 1) / 8 + 2));
}

static UnityEngine::UI::Button *editServerButton = NULL;
static void InvalidateModels();
static void RefreshNetworkConfig() {
	editServerButton->set_interactable(false);
	if(customNetworkConfig) {
		GlobalNamespace::NetworkConfigSO *networkConfigPrefab = mainSystemInit->networkConfig;
		std::string hostname = to_utf8(csstrtostr(networkConfigPrefab->get_masterServerEndPoint()->hostName));
		int32_t port = networkConfigPrefab->get_masterServerEndPoint()->port;
		bool forceGameLift = networkConfigPrefab->get_forceGameLift();
		std::string multiplayerStatusUrl = to_utf8(csstrtostr(networkConfigPrefab->get_multiplayerStatusUrl()));
		if(std::string customHostName = to_utf8(csstrtostr(customServerHostName->get_value())); customHostName.length()) {
			editServerButton->set_interactable(true);
			hostname = to_utf8(csstrtostr(customServerHostName->get_value()->ToLower()));
			if(std::string_view::size_type pos = hostname.find(':'); pos != std::string::npos) {
				port = std::stoi(hostname.substr(pos + 1));
				hostname.resize(pos);
			}
			forceGameLift = false;
			multiplayerStatusUrl = "";
			for(std::pair<std::string, std::string> &e : Config::Servers) {
				if(e.first == customHostName) {
					multiplayerStatusUrl = e.second;
					break;
				}
			}
		}
		std::u16string_view oldMultiplayerStatusUrl = csstrtostr(customNetworkConfig->get_multiplayerStatusUrl());
		logger->info("CustomNetworkConfig(customServerHostName=\"%s\", port=%d, forceGameLift=%s), multiplayerStatusUrl=\"%s\"", hostname.c_str(), port, forceGameLift ? "true" : "false", multiplayerStatusUrl.c_str());
		il2cpp_utils::RunMethod(customNetworkConfig, ".ctor", il2cpp_utils::cast<GlobalNamespace::INetworkConfig>(networkConfigPrefab), StringW(hostname), port, forceGameLift);
		if(multiplayerStatusUrl.length())
			customNetworkConfig->multiplayerStatusUrl = multiplayerStatusUrl;
		if(!forceGameLift)
			customNetworkConfig->serviceEnvironment = networkConfigPrefab->get_serviceEnvironment();
		if(csstrtostr(customNetworkConfig->get_multiplayerStatusUrl()) != oldMultiplayerStatusUrl)
			InvalidateModels();
	}
}

static BeatUpClient::UI::DropdownSetting *serverDropdown = NULL;
static inline void RefreshDropdown() {
	serverDropdown->SetOptions(Config::Servers);
}

DEFINE_CLASS_CODEGEN(BeatUpClient, EditServerViewController, HMUI::ViewController,
	HMUI::FlowCoordinator *flowCoordinator;
	bool edit;
	VRUIControls::VRInputModule *vrInputModule;
	HMUI::InputFieldView *editHostnameTextbox;
	HMUI::InputFieldView *editStatusTextbox;
	HMUI::CurvedTextMeshPro *editStatusPlaceholder;
	HMUI::UIKeyboard *keyboard;
	UnityEngine::UI::Button *cancelButton;
	std::string activeKey;
	System::Action *HandleOkButtonWasPressed_delegate;
	System::Action_1<UnityEngine::GameObject*> *ProcessMousePress_delegate;

	static EditServerViewController *Create(const char *name, UnityEngine::Transform *parent) {
		UnityEngine::GameObject *gameObject = il2cpp_utils::cast<UnityEngine::GameObject>(Object_Internal_CloneSingleWithParent(CRASH_UNLESS(FindResourceOfType<GlobalNamespace::ServerCodeEntryViewController>())->get_gameObject(), parent, false));
		Object_SetName(gameObject, StringW(std::string_view(name)));
		Object_Destroy(GameObject_GetComponent(gameObject, csTypeOf(GlobalNamespace::ServerCodeEntryViewController*)), 0);
		Object_Destroy(GameObject_GetComponentInChildren(gameObject, csTypeOf(HMUI::InputFieldView*), false)->get_gameObject(), 0);
		EditServerViewController *viewController = gameObject->AddComponent<EditServerViewController*>();
		viewController->flowCoordinator = NULL;
		viewController->edit = false;
		UnityEngine::RectTransform *rectTransform = viewController->get_rectTransform();
		rectTransform->set_anchorMin(UnityEngine::Vector2(0, 0));
		rectTransform->set_anchorMax(UnityEngine::Vector2(1, 1));
		rectTransform->set_sizeDelta(UnityEngine::Vector2(0, 0));
		rectTransform->set_anchoredPosition(UnityEngine::Vector2(0, 0));
		viewController->vrInputModule = CRASH_UNLESS(FindResourceOfType<VRUIControls::VRInputModule>());

		UnityEngine::Transform *Wrapper = gameObject->get_transform()->GetChild(0);
		UnityEngine::RectTransform *editHostname = UI::CreateTextbox(Wrapper, "EditHostname", 1, &EditServerViewController::RefreshStatusPlaceholder, viewController, "BEATUP_ENTER_HOSTNAME");
		UnityEngine::RectTransform *editStatus = UI::CreateTextbox(Wrapper, "EditStatus", 2, &EditServerViewController::RefreshStatusPlaceholder, viewController);
		editHostname->set_sizeDelta(UnityEngine::Vector2(80, editHostname->get_sizeDelta().y));
		editHostname->set_localPosition(UnityEngine::Vector3(0, -1.5f, 0));
		editStatus->set_sizeDelta(UnityEngine::Vector2(80, editStatus->get_sizeDelta().y));
		editStatus->set_localPosition(UnityEngine::Vector3(0, -8.5f, 0));
		viewController->editHostnameTextbox = ::GetComponent<HMUI::InputFieldView>(editHostname->get_gameObject());
		viewController->editStatusTextbox = ::GetComponent<HMUI::InputFieldView>(editStatus->get_gameObject());
		viewController->editHostnameTextbox->textLengthLimit = 41;
		viewController->editStatusTextbox->textLengthLimit = 41;
		viewController->editStatusPlaceholder = ::GetComponent<HMUI::CurvedTextMeshPro>(viewController->editStatusTextbox->placeholderText);
		viewController->cancelButton = ::GetComponent<HMUI::NoTransitionsButton>(Wrapper->Find("Buttons/CancelButton"sv)->get_gameObject());

		viewController->keyboard = il2cpp_utils::cast<HMUI::UIKeyboard>(GameObject_GetComponentInChildren(gameObject, csTypeOf(HMUI::UIKeyboard*), false));
		for(UnityEngine::Transform *tr : {viewController->keyboard->get_transform()->get_parent(), viewController->keyboard->get_transform()}) {
			UnityEngine::RectTransform *rtr = il2cpp_utils::cast<UnityEngine::RectTransform>(tr);
			rtr->set_sizeDelta(UnityEngine::Vector2(rtr->get_sizeDelta().x + 7, rtr->get_sizeDelta().y));
		}
		UI::AddKey(viewController->keyboard, false, 0, -1, UnityEngine::KeyCode::Underscore, u'_');
		UI::AddKey(viewController->keyboard, false, 1, -1, UnityEngine::KeyCode::Colon, u':');
		UI::AddKey(viewController->keyboard, false, 2, -2, UnityEngine::KeyCode::Slash, u'/');
		UnityEngine::GameObject *dot = UI::AddKey(viewController->keyboard, true, 3, -1, UnityEngine::KeyCode::Period, u'.');
		il2cpp_utils::cast<UnityEngine::RectTransform>(dot->get_transform()->get_parent()->GetChild(0))->set_sizeDelta(UnityEngine::Vector2(14, 7));

		GameObject_SetActive(gameObject, false);
		return viewController;
	}

	DEFINE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::il2cpp_type_check::MetadataGetter<&HMUI::ViewController::DidActivate>::get(), (bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
		if(firstActivation)
			get_buttonBinder()->AddBinding(cancelButton, MakeDelegate<System::Action*>(&EditServerViewController::Dismiss, this));
		activeKey = "";
		std::string_view status = "";
		std::string hostname = to_utf8(csstrtostr(customServerHostName->get_value()));
		if(edit) {
			for(std::pair<std::string, std::string> &e : Config::Servers) {
				if(e.first == hostname) {
					activeKey = hostname;
					status = e.second;
					break;
				}
			}
		}
		editHostnameTextbox->SetText(activeKey);
		if(HMUI::InputFieldView::InputFieldChanged *onValueChanged = editHostnameTextbox->get_onValueChanged(); onValueChanged)
			onValueChanged->Invoke(editHostnameTextbox);
		editStatusTextbox->SetText(status);
		editHostnameTextbox->ActivateKeyboard(keyboard);
		editHostnameTextbox->UpdateClearButton();
		keyboard->add_okButtonWasPressedEvent(HandleOkButtonWasPressed_delegate = MakeDelegate<System::Action*>(&EditServerViewController::HandleOkButtonWasPressed, this));
		vrInputModule->add_onProcessMousePressEvent(ProcessMousePress_delegate = MakeDelegate<System::Action_1<UnityEngine::GameObject*>*>(&EditServerViewController::ProcessMousePress, this));
	});

	DEFINE_OVERRIDE_METHOD(void, DidDeactivate, il2cpp_utils::il2cpp_type_check::MetadataGetter<&HMUI::ViewController::DidDeactivate>::get(), (bool removedFromHierarchy, bool screenSystemDisabling) {
		vrInputModule->remove_onProcessMousePressEvent(ProcessMousePress_delegate);
		keyboard->remove_okButtonWasPressedEvent(HandleOkButtonWasPressed_delegate);
		editStatusTextbox->DeactivateKeyboard(keyboard);
		editHostnameTextbox->DeactivateKeyboard(keyboard);
		flowCoordinator = NULL;
	});
	public:
	void ProcessMousePress(UnityEngine::GameObject *gameObject) {
		HMUI::InputFieldView *inputField = ::TryGetComponent<HMUI::InputFieldView>(gameObject);
		if(inputField == editHostnameTextbox) {
			editStatusTextbox->DeactivateKeyboard(keyboard);
			editHostnameTextbox->ActivateKeyboard(keyboard);
			editHostnameTextbox->UpdateClearButton();
		} else if(inputField == editStatusTextbox) {
			editHostnameTextbox->DeactivateKeyboard(keyboard);
			editStatusTextbox->ActivateKeyboard(keyboard);
			editStatusTextbox->UpdateClearButton();
		}
	}

	void RefreshStatusPlaceholder(HMUI::InputFieldView *textbox) {
		std::string text = to_utf8(csstrtostr(editHostnameTextbox->get_text()));
		if(std::string::size_type pos = text.find(':'); pos != std::string::npos)
			text.resize(pos);
		bool enable = text.length() >= 1;
		if(enable != editStatusTextbox->get_gameObject()->get_activeSelf()) {
			GameObject_SetActive(editStatusTextbox->get_gameObject(), enable);
			if(!enable)
				editStatusTextbox->DeactivateKeyboard(keyboard);
		}
		if(enable)
			editStatusPlaceholder->set_text("https://status."sv + text);
	}

	void TryPresent(HMUI::FlowCoordinator *flowCoordinator, bool edit) {
		if(!this->flowCoordinator && il2cpp_utils::try_cast<GlobalNamespace::MultiplayerModeSelectionFlowCoordinator>(flowCoordinator).value_or(nullptr)) {
			this->flowCoordinator = flowCoordinator;
			this->edit = edit;
			flowCoordinator->PresentViewController(this, NULL, HMUI::ViewController::AnimationDirection::Vertical, false);
			flowCoordinator->SetTitle(Polyglot::Localization::Get(edit ? "BEATUP_EDIT_SERVER"sv : "BEATUP_ADD_SERVER"sv), HMUI::ViewController::AnimationType::In);
			flowCoordinator->screenSystem->SetBackButton(false, true);
		}
	}

	void Dismiss(bool immediately = false) {
		if(flowCoordinator)
			flowCoordinator->DismissViewController(this, HMUI::ViewController::AnimationDirection::Vertical, NULL, immediately);
	}

	void HandleOkButtonWasPressed() {
		if(activeKey.length()) {
			for(std::vector<std::pair<std::string, std::string>>::iterator it = Config::Servers.begin(); it != Config::Servers.end();) {
				if(it->first == activeKey)
					it = Config::Servers.erase(it);
				else
					++it;
			}
		}
		std::string hostname = to_utf8(csstrtostr(editHostnameTextbox->get_text()));
		if(hostname.length() && hostname.find(':')) {
			std::string status = to_utf8(csstrtostr(editStatusTextbox->get_text()));
			std::vector<std::pair<std::string, std::string>>::iterator it;
			for(it = Config::Servers.begin(); it != Config::Servers.end(); ++it) {
				if(it->first == hostname) {
					it->second = status;
					break;
				}
			}
			if(it == Config::Servers.end())
				Config::Servers.push_back(std::pair(hostname, status));
		} else {
			hostname = "";
		}
		if(to_utf8(csstrtostr(customServerHostName->get_value())) == hostname)
			RefreshNetworkConfig();
		customServerHostName->set_value(hostname);
		RefreshDropdown();
		Dismiss();
	}
)

static BeatUpClient::EditServerViewController *editServerViewController = NULL;
static void InvalidateModels() {
	if(multiplayerStatusModel)
		multiplayerStatusModel->request = NULL;
	if(quickPlaySetupModel)
		quickPlaySetupModel->request = NULL;
	if(HMUI::FlowCoordinator *childFlowCoordinator = mainFlowCoordinator->get_childFlowCoordinator(); il2cpp_utils::try_cast<GlobalNamespace::MultiplayerModeSelectionFlowCoordinator>(childFlowCoordinator).value_or(nullptr)) {
		editServerViewController->Dismiss(true);
		mainFlowCoordinator->DismissFlowCoordinator(childFlowCoordinator, HMUI::ViewController::AnimationDirection::Horizontal, MakeDelegate<System::Action*>((void(*)())[]() {
			mainFlowCoordinator->PresentMultiplayerModeSelectionFlowCoordinatorWithDisclaimerAndAvatarCreator();
		}), true);
	}
}

static void OnSceneLoaded(UnityEngine::SceneManagement::Scene scene, UnityEngine::SceneManagement::LoadSceneMode mode) {
	IL2CPP_CATCH_HANDLER(
	if(to_utf8(csstrtostr(scene.get_name())) != "MainMenu")
		return;
	logger->info("load MainMenu");
	if(customNetworkConfig) {
		logger->info("CustomNetworkConfig active");
		UnityEngine::Transform *CreateServerFormView = CRASH_UNLESS(FindResourceOfType<GlobalNamespace::CreateServerFormController>())->get_transform();
		BeatUpClient::UI::CreateValuePicker(CreateServerFormView, "CountdownDuration", "BEATUP_COUNTDOWN_DURATION", &Config::CountdownDuration, {uint8_t(Config::CountdownDuration * 4), 0, 12, 20, 32, 40, 60});
		BeatUpClient::UI::CreateToggle(CreateServerFormView, "SkipResults", "BEATUP_SKIP_RESULTS_PYRAMID", &Config::SkipResults);
		BeatUpClient::UI::CreateToggle(CreateServerFormView, "PerPlayerDifficulty", "BEATUP_PER_PLAYER_DIFFICULTY", &Config::PerPlayerDifficulty);
		BeatUpClient::UI::CreateToggle(CreateServerFormView, "PerPlayerModifiers", "BEATUP_PER_PLAYER_MODIFIERS", &Config::PerPlayerModifiers);
		GetComponent<UnityEngine::UI::VerticalLayoutGroup>(CreateServerFormView->get_parent()->get_gameObject())->set_enabled(true);
		GetComponent<UnityEngine::UI::VerticalLayoutGroup>(CreateServerFormView->get_gameObject())->set_enabled(true);
		GetComponent<UnityEngine::UI::ContentSizeFitter>(CreateServerFormView->get_gameObject())->set_enabled(true);
		GameObject_SetActive(CreateServerFormView->get_parent()->get_parent()->get_gameObject(), true);

		mainFlowCoordinator = CRASH_UNLESS(FindResourceOfType<GlobalNamespace::MainFlowCoordinator>());
		GlobalNamespace::MultiplayerModeSelectionViewController *multiplayerModeSelectionViewController = CRASH_UNLESS(FindResourceOfType<GlobalNamespace::MultiplayerModeSelectionViewController>());
		TMPro::TextMeshProUGUI *customServerEndPointText = multiplayerModeSelectionViewController->customServerEndPointText;
		UnityEngine::UI::Button *editColorSchemeButton = CRASH_UNLESS(FindResourceOfType<GlobalNamespace::ColorsOverrideSettingsPanelController>())->editColorSchemeButton;
		customServerEndPointText->set_enabled(false);
		if(!editServerViewController)
			editServerViewController = BeatUpClient::EditServerViewController::Create("BeatUpClient_EditServerView", multiplayerModeSelectionViewController->get_transform()->get_parent());
		UnityEngine::RectTransform *server = BeatUpClient::UI::CreateSimpleDropdown(customServerEndPointText->get_transform(), "Server", customServerHostName, Config::Servers);
		server->set_sizeDelta(UnityEngine::Vector2(80, server->get_sizeDelta().y));
		server->set_localPosition(UnityEngine::Vector3(0, 39.5f, 0));
		for(int32_t i = 0, childCount = server->get_childCount(); i < childCount; ++i)
			server->GetChild(i)->set_localPosition(UnityEngine::Vector3(0, 0, 0));
		serverDropdown = GetComponent<BeatUpClient::UI::DropdownSetting>(server->get_gameObject());
		UnityEngine::RectTransform *addButton = BeatUpClient::UI::CreateButtonFrom(editColorSchemeButton->get_gameObject(), customServerEndPointText->get_transform(), "AddServer", []() {
			editServerViewController->TryPresent(mainFlowCoordinator->get_childFlowCoordinator(), false);
		});
		addButton->set_localPosition(UnityEngine::Vector3(-40, 39.5f, 0));
		for(UnityEngine::Sprite *sprite : FindResourcesOfType<UnityEngine::Sprite>()) {
			if(to_utf8(csstrtostr(sprite->get_name())) == "AddIcon") {
				GetComponent<HMUI::ImageView>(addButton->Find("Icon"sv)->get_gameObject())->set_sprite(sprite);
				break;
			}
		}
		UnityEngine::RectTransform *editButton = BeatUpClient::UI::CreateButtonFrom(editColorSchemeButton->get_gameObject(), customServerEndPointText->get_transform(), "EditServer", []() {
			editServerViewController->TryPresent(mainFlowCoordinator->get_childFlowCoordinator(), true);
		});
		editButton->set_localPosition(UnityEngine::Vector3(52, 39.5f, 0));
		editServerButton = GetComponent<UnityEngine::UI::Button>(editButton->get_gameObject());

		static System::Action *RefreshNetworkConfig_delegate = MakeDelegate<System::Action*>(RefreshNetworkConfig);
		customServerHostName->remove_didChangeEvent(RefreshNetworkConfig_delegate);
		customServerHostName->add_didChangeEvent(RefreshNetworkConfig_delegate);
		RefreshNetworkConfig();
	} else {
		logger->warning("CustomNetworkConfig inactive");
	}

	UnityEngine::Transform *leftPanel = CRASH_UNLESS(FindResourceOfType<GlobalNamespace::MultiplayerSettingsPanelController>())->get_transform();
	infoText = il2cpp_utils::cast<UnityEngine::GameObject>(Object_Internal_CloneSingleWithParent(CRASH_UNLESS(leftPanel->get_parent()->Find("PlayerOptions/ViewPort/Content/SinglePlayerOnlyTitle"sv))->get_gameObject(), leftPanel, false));
	Object_SetName(infoText, StringW("BeatUpClient_Info"sv));
	Polyglot::LocalizedTextMeshProUGUI *text = il2cpp_utils::cast<Polyglot::LocalizedTextMeshProUGUI>(GameObject_GetComponentInChildren(infoText, csTypeOf(Polyglot::LocalizedTextMeshProUGUI*), false));
	text->localizedComponent->set_richText(true);
	text->set_Key("BEATUP_INFO"sv);
	)

	// TODO: Uncaught C++ exception! type name: NSt6__ndk117bad_function_callE, what(): std::exception
	#ifndef DISABLE_SELECTORS
	DifficultyPanel::Init();
	lobbyDifficultyPanel.Configure(CRASH_UNLESS(FindResourceOfType<GlobalNamespace::LobbySetupViewController>())->get_transform()->GetChild(0), 2, 90);
	#endif
}

extern "C" DL_EXPORT void setup(ModInfo& info) {
	info.id = "BeatUpClient";
	info.version = VERSION;
	// new(&config) Configuration(info);
	logger = new Logger(info, LoggerOptions(false, true));
	// config.Load();
	logger->info("Completed setup!");
}

bool (*ResolveICalls)(Logger *logger) = [](Logger*) {
	logger->info("ResolveICalls()");
	return false;
};
void (*InstallHooks)(Logger *logger) = [](Logger*) {
	logger->info("InstallHooks()");
};

extern "C" DL_EXPORT void load() {
	if(GlobalNamespace::NetworkConstants::_get_kProtocolVersion() != 8) {
		logger->critical("Unsupported game version!");
		return;
	}
	logger->info("loading @ %.*s", 2, &__TIME__[3]);
	il2cpp_functions::Init();
	if(ResolveICalls(logger))
		return;
	InstallHooks(logger);
	logger->info("custom_types::Register::AutoRegister()");
	custom_types::Register::AutoRegister();

	#ifndef DISABLE_LOCALIZATION
	logger->info("Polyglot::LocalizationImporter::Import()");
	std::string localization = "Polyglot\t100\n"
		"BEATUP_COUNTDOWN_DURATION\t\tCountdown Duration\t"/*French*/"\t"/*Spanish*/"\t"/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"/*Japanese*/"\t"/*Simplified Chinese*/"\t\t"/*Korean*/"\t\t\t\t\t\t\t\t\n"
		"BEATUP_SKIP_RESULTS_PYRAMID\t\tSkip Results Pyramid\t"/*French*/"\t"/*Spanish*/"\t"/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"/*Japanese*/"\t"/*Simplified Chinese*/"\t\t"/*Korean*/"\t\t\t\t\t\t\t\t\n"
		"BEATUP_PER_PLAYER_DIFFICULTY\t\tPer-Player Difficulty\t"/*French*/"\t"/*Spanish*/"\t"/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"/*Japanese*/"\t"/*Simplified Chinese*/"\t\t"/*Korean*/"\t\t\t\t\t\t\t\t\n"
		"BEATUP_PER_PLAYER_MODIFIERS\t\tPer-Player Modifiers\t"/*French*/"\t"/*Spanish*/"\t"/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"/*Japanese*/"\t"/*Simplified Chinese*/"\t\t"/*Korean*/"\t\t\t\t\t\t\t\t\n"
		"BEATUP_ADD_SERVER\t\tAdd Server\t"/*French*/"\t"/*Spanish*/"\t"/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"/*Japanese*/"\t"/*Simplified Chinese*/"\t\t"/*Korean*/"\t\t\t\t\t\t\t\t\n"
		"BEATUP_EDIT_SERVER\t\tEdit Server\t"/*French*/"\t"/*Spanish*/"\t"/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"/*Japanese*/"\t"/*Simplified Chinese*/"\t\t"/*Korean*/"\t\t\t\t\t\t\t\t\n"
		"BEATUP_ENTER_HOSTNAME\t\tEnter Hostname\t"/*French*/"\t"/*Spanish*/"\t"/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"/*Japanese*/"\t"/*Simplified Chinese*/"\t\t"/*Korean*/"\t\t\t\t\t\t\t\t\n"
		"BEATUP_INFO\t\tBeatUpClient " VERSION " <color=red>| ALPHA</color> is active.<br>If any issues arise, please contact rcelyte#5372 <b>immediately</b>.\t"/*French*/"\t"/*Spanish*/"\t"/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"/*Japanese*/"\t"/*Simplified Chinese*/"\t\t"/*Korean*/"\t\t\t\t\t\t\t\t\n"
		"BEATUP_VOTE_MODIFIERS\t\tSuggested Modifiers (Vote)\t"/*French*/"\t"/*Spanish*/"\t"/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"/*Japanese*/"\t"/*Simplified Chinese*/"\t\t"/*Korean*/"\t\t\t\t\t\t\t\t\n"
		"BEATUP_SWITCH\t\tSwitch\t"/*French*/"\t"/*Spanish*/"\t"/*German*/"\t\t\t\t\t\t\t\t\t\t\t\t\t"/*Japanese*/"\t"/*Simplified Chinese*/"\t\t"/*Korean*/"\t\t\t\t\t\t\t\t\n";
	Polyglot::LocalizationImporter::Import(localization, Polyglot::GoogleDriveDownloadFormat::TSV); // TODO: SIGSEGV (tombstone_00)
	#endif
	logger->info("UnityEngine::SceneManagement::SceneManager::add_sceneLoaded()");
	UnityEngine::SceneManagement::SceneManager::add_sceneLoaded(MakeDelegate<UnityEngine::Events::UnityAction_2<UnityEngine::SceneManagement::Scene, UnityEngine::SceneManagement::LoadSceneMode>*>(OnSceneLoaded));
	logger->info("DONE LOADING");
}
