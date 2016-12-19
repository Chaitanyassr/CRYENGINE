// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "ATLUtils.h"
#include "AudioInternalInterfaces.h"
#include <SharedAudioData.h>
#include <CrySystem/IStreamEngine.h>
#include <CrySystem/TimeValue.h>
#include <PoolObject.h>
#include <CryString/HashedString.h>

namespace CryAudio
{
class CATLAudioObject;

struct SATLXMLTags
{
	static char const* const szPlatform;

	static char const* const szRootNodeTag;
	static char const* const szEditorDataTag;
	static char const* const szTriggersNodeTag;
	static char const* const szParametersNodeTag;
	static char const* const szSwitchesNodeTag;
	static char const* const szPreloadsNodeTag;
	static char const* const szEnvironmentsNodeTag;

	static char const* const szATLTriggerTag;
	static char const* const szATLSwitchTag;
	static char const* const szATLParametersTag;
	static char const* const szATLSwitchStateTag;
	static char const* const szATLEnvironmentTag;
	static char const* const szATLPlatformsTag;
	static char const* const szATLConfigGroupTag;

	static char const* const szATLTriggerRequestTag;
	static char const* const szATLSwitchRequestTag;
	static char const* const szATLValueTag;
	static char const* const szATLParametersRequestTag;
	static char const* const szATLPreloadRequestTag;
	static char const* const szATLEnvironmentRequestTag;

	static char const* const szATLVersionAttribute;
	static char const* const szATLNameAttribute;
	static char const* const szATLInternalNameAttribute;
	static char const* const szATLTypeAttribute;
	static char const* const szATLConfigGroupAttribute;
	static char const* const szATLRadiusAttribute;
	static char const* const szATLOcclusionFadeOutDistanceAttribute;

	static char const* const szATLDataLoadType;
};

struct SATLInternalControlIDs
{
	static ControlId        obstructionOcclusionCalcSwitchId;
	static ControlId        objectDopplerTrackingSwitchId;
	static ControlId        objectVelocityTrackingSwitchId;
	static ControlId        loseFocusTriggerId;
	static ControlId        getFocusTriggerId;
	static ControlId        muteAllTriggerId;
	static ControlId        unmuteAllTriggerId;
	static ControlId        objectDopplerParameterId;
	static ControlId        objectVelocityParameterId;
	static SwitchStateId    ignoreStateId;
	static SwitchStateId    adaptiveStateId;
	static SwitchStateId    lowStateId;
	static SwitchStateId    mediumStateId;
	static SwitchStateId    highStateId;
	static SwitchStateId    onStateId;
	static SwitchStateId    offStateId;
	static PreloadRequestId globalPreloadRequestId;
};

void InitATLControlIDs();   // initializes the values in SATLInternalControlIDs

namespace Impl
{
struct IAudioObject;
struct IAudioListener;
struct IAudioTrigger;
struct IParameter;
struct IAudioSwitchState;
struct IAudioEnvironment;
struct IAudioEvent;
struct IAudioFileEntry;
struct IAudioImpl;
} // namespace Impl

enum EAudioObjectFlags : EnumFlagsType
{
	eAudioObjectFlags_None                            = 0,
	eAudioObjectFlags_TrackDoppler                    = BIT(0),
	eAudioObjectFlags_TrackVelocity                   = BIT(1),
	eAudioObjectFlags_NeedsDopplerUpdate              = BIT(2),
	eAudioObjectFlags_NeedsVelocityUpdate             = BIT(3),
	eAudioObjectFlags_DoNotRelease                    = BIT(4),
	eAudioObjectFlags_Virtual                         = BIT(5),
	eAudioObjectFlags_WaitingForInitialTransformation = BIT(6),
};

enum EAudioFileFlags : EnumFlagsType
{
	eAudioFileFlags_Cached                    = BIT(0),
	eAudioFileFlags_NotCached                 = BIT(1),
	eAudioFileFlags_NotFound                  = BIT(2),
	eAudioFileFlags_MemAllocFail              = BIT(3),
	eAudioFileFlags_Removable                 = BIT(4),
	eAudioFileFlags_Loading                   = BIT(5),
	eAudioFileFlags_UseCounted                = BIT(6),
	eAudioFileFlags_NeedsResetToManualLoading = BIT(7),
	eAudioFileFlags_Localized                 = BIT(8),
};

template<typename IDType>
class CATLEntity
{
public:

	explicit CATLEntity(IDType const id, EDataScope const dataScope)
		: m_id(id)
		, m_dataScope(dataScope)
	{}

	CATLEntity(CATLEntity const&) = delete;
	CATLEntity(CATLEntity&&) = delete;
	CATLEntity&        operator=(CATLEntity const&) = delete;
	CATLEntity&        operator=(CATLEntity&&) = delete;

	virtual IDType     GetId() const        { return m_id; }
	virtual EDataScope GetDataScope() const { return m_dataScope; }

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	CryFixedStringT<MaxControlNameLength> m_name;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

protected:

	~CATLEntity() = default;
	EDataScope m_dataScope;

private:

	IDType const m_id;

};

typedef CATLEntity<ControlId> ATLControl;

struct SATLSoundPropagationData
{
	float obstruction = 0.0f;
	float occlusion = 0.0f;
};

class CATLListener final : public IListener
{
public:

	CATLListener(CATLListener const&) = delete;
	CATLListener(CATLListener&&) = delete;
	CATLListener& operator=(CATLListener const&) = delete;
	CATLListener& operator=(CATLListener&&) = delete;

	explicit CATLListener(Impl::IAudioListener* const pImplData)
		: m_pImplData(pImplData)
		, m_bNeedsFinalSetPosition(false)
	{}

	// CryAudio::IListener
	virtual void SetTransformation(CObjectTransformation const& transformation, SRequestUserData const& userData = SRequestUserData::GetEmptyObject()) override;
	// ~CryAudio::IListener

	void                             Update();
	ERequestStatus                   HandleSetTransformation(CObjectTransformation const& transformation);
	Impl::SObject3DAttributes const& Get3DAttributes() const { return m_attributes; }

	Impl::IAudioListener* m_pImplData;

private:

	bool                      m_bNeedsFinalSetPosition;
	Impl::SObject3DAttributes m_attributes;
	Impl::SObject3DAttributes m_previousAttributes;
	CTimeValue                m_previousTime;

};

class CATLControlImpl
{

public:
	CATLControlImpl() = default;
	CATLControlImpl(CATLControlImpl const&) = delete;
	CATLControlImpl(CATLControlImpl&&) = delete;
	CATLControlImpl& operator=(CATLControlImpl const&) = delete;
	CATLControlImpl& operator=(CATLControlImpl&&) = delete;

	static void      SetImpl(Impl::IAudioImpl* const pImpl) { s_pImpl = pImpl; }

protected:
	static Impl::IAudioImpl* s_pImpl;
};

class CATLTriggerImpl final : public CATLControlImpl
{
public:

	explicit CATLTriggerImpl(
	  TriggerImplId const audioTriggerImplId,
	  Impl::IAudioTrigger const* const pImplData = nullptr)
		: m_audioTriggerImplId(audioTriggerImplId)
		, m_pImplData(pImplData)
	{}

	~CATLTriggerImpl()
	{
		CRY_ASSERT(s_pImpl != nullptr);
		s_pImpl->DeleteAudioTrigger(m_pImplData);
	}

	TriggerImplId const              m_audioTriggerImplId;
	Impl::IAudioTrigger const* const m_pImplData;
};

class CATLTrigger final : public ATLControl
{
public:

	typedef std::vector<CATLTriggerImpl const*> ImplPtrVec;

	explicit CATLTrigger(
	  ControlId const audioTriggerId,
	  EDataScope const dataScope,
	  ImplPtrVec const& implPtrs,
	  float const maxRadius,
	  float const occlusionFadeOutDistance)
		: ATLControl(audioTriggerId, dataScope)
		, m_implPtrs(implPtrs)
		, m_maxRadius(maxRadius)
		, m_occlusionFadeOutDistance(occlusionFadeOutDistance)
	{}

	ImplPtrVec const m_implPtrs;
	float const      m_maxRadius;
	float const      m_occlusionFadeOutDistance;
};

// Base class for a parameter implementation
class IParameterImpl : public CATLControlImpl
{
public:
	virtual ~IParameterImpl() = default;
	virtual ERequestStatus Set(CATLAudioObject& audioObject, float const value) const = 0;
};

// Class for a parameter associated with a middleware parameter
class CParameterImpl final : public IParameterImpl
{
public:

	explicit CParameterImpl(Impl::IParameter const* const pImplData = nullptr)
		: m_pImplData(pImplData)
	{}

	~CParameterImpl()
	{
		CRY_ASSERT(s_pImpl != nullptr);
		s_pImpl->DeleteAudioParameter(m_pImplData);
	}

	virtual ERequestStatus Set(CATLAudioObject& audioObject, float const value) const override;

private:

	Impl::IParameter const* const m_pImplData = nullptr;
};

class CParameter final : public ATLControl
{
public:

	typedef std::vector<IParameterImpl const*> ImplPtrVec;

	explicit CParameter(ControlId const parameterId, EDataScope const dataScope, ImplPtrVec const& cImplPtrs)
		: ATLControl(parameterId, dataScope)
		, m_implPtrs(cImplPtrs)
	{}

	ImplPtrVec const m_implPtrs;
};

class IAudioSwitchStateImpl : public CATLControlImpl
{
public:
	virtual ~IAudioSwitchStateImpl() = default;
	virtual ERequestStatus Set(CATLAudioObject& audioObject) const = 0;
};

class CExternalAudioSwitchStateImpl final : public IAudioSwitchStateImpl
{
public:

	explicit CExternalAudioSwitchStateImpl(Impl::IAudioSwitchState const* const pImplData)
		: m_pImplData(pImplData)
	{}

	~CExternalAudioSwitchStateImpl()
	{
		CRY_ASSERT(s_pImpl != nullptr);
		s_pImpl->DeleteAudioSwitchState(m_pImplData);
	}

	// IAudioSwitchStateImpl
	virtual ERequestStatus Set(CATLAudioObject& audioObject) const override;
	// ~IAudioSwitchStateImpl

private:

	Impl::IAudioSwitchState const* const m_pImplData;
};

class CATLSwitchState final
{
public:

	typedef std::vector<IAudioSwitchStateImpl const*> ImplPtrVec;

	explicit CATLSwitchState(
	  ControlId const audioSwitchId,
	  SwitchStateId const audioSwitchStateId,
	  ImplPtrVec const& implPtrs)
		: m_audioSwitchStateId(audioSwitchStateId)
		, m_audioSwitchId(audioSwitchId)
		, m_implPtrs(implPtrs)
	{}

	CATLSwitchState(CATLSwitchState const&) = delete;
	CATLSwitchState(CATLSwitchState&&) = delete;
	CATLSwitchState& operator=(CATLSwitchState const&) = delete;
	CATLSwitchState& operator=(CATLSwitchState&&) = delete;

	SwitchStateId    GetId() const       { return m_audioSwitchStateId; }
	SwitchStateId    GetParentId() const { return m_audioSwitchId; }

	ImplPtrVec const m_implPtrs;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	CryFixedStringT<MaxControlNameLength> m_name;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

private:

	SwitchStateId const m_audioSwitchStateId;
	ControlId const     m_audioSwitchId;
};

class CATLSwitch final : public ATLControl
{
public:

	explicit CATLSwitch(ControlId const audioSwitchId, EDataScope const dataScope)
		: ATLControl(audioSwitchId, dataScope)
	{}

	typedef std::map<SwitchStateId, CATLSwitchState const*> AudioStates;
	AudioStates audioSwitchStates;
};

class CATLEnvironmentImpl final : public CATLControlImpl
{
public:

	explicit CATLEnvironmentImpl(Impl::IAudioEnvironment const* const pImplData)
		: m_pImplData(pImplData)
	{}

	~CATLEnvironmentImpl()
	{
		CRY_ASSERT(s_pImpl != nullptr);
		s_pImpl->DeleteAudioEnvironment(m_pImplData);
	}

	Impl::IAudioEnvironment const* const m_pImplData;
};

class CATLAudioEnvironment final : public CATLEntity<EnvironmentId>
{
public:

	typedef std::vector<CATLEnvironmentImpl const*> ImplPtrVec;

	explicit CATLAudioEnvironment(EnvironmentId const audioEnvironmentId, EDataScope const dataScope, ImplPtrVec const& implPtrs)
		: CATLEntity<EnvironmentId>(audioEnvironmentId, dataScope)
		, m_implPtrs(implPtrs)
	{}

	ImplPtrVec const m_implPtrs;
};

class CATLStandaloneFile final : public CPoolObject<CATLStandaloneFile>
{
public:

	explicit CATLStandaloneFile()
	{}

	bool IsPlaying() const { return (m_state == eAudioStandaloneFileState_Playing) || (m_state == eAudioStandaloneFileState_Stopping); }

	CATLAudioObject*            m_pAudioObject = nullptr;
	Impl::IAudioStandaloneFile* m_pImplData = nullptr;
	EAudioStandaloneFileState   m_state = eAudioStandaloneFileState_None;
	CHashedString               m_hashedFilename;

	// These variables are only needed when switching middleware
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	bool                       m_bLocalized = true;
	void*                      m_pOwner = nullptr;
	void*                      m_pUserData = nullptr;
	void*                      m_pUserDataOwner = nullptr;
	Impl::IAudioTrigger const* m_pTrigger = nullptr;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

};

class CATLEvent final : public CPoolObject<CATLEvent>
{
public:
	ERequestStatus Reset();
	ERequestStatus Stop();
	void           SetDataScope(EDataScope const dataScope) { m_dataScope = dataScope; }
	bool           IsPlaying() const                        { return m_audioEventState == eAudioEventState_Playing || m_audioEventState == eAudioEventState_PlayingDelayed; }

	EDataScope         m_dataScope = eDataScope_None;
	CATLAudioObject*   m_pAudioObject = nullptr;
	CATLTrigger const* m_pTrigger = nullptr;
	TriggerImplId      m_audioTriggerImplId = InvalidTriggerImplId;
	TriggerInstanceId  m_audioTriggerInstanceId = InvalidTriggerInstanceId;
	EAudioEventState   m_audioEventState = eAudioEventState_None;
	Impl::IAudioEvent* m_pImplData = nullptr;
};

class CATLAudioFileEntry final
{
public:

	explicit CATLAudioFileEntry(char const* const szPath = nullptr, Impl::IAudioFileEntry* const pImplData = nullptr)
		: m_path(szPath)
		, m_size(0)
		, m_useCount(0)
		, m_memoryBlockAlignment(MEMORY_ALLOCATION_ALIGNMENT)
		, m_flags(eAudioFileFlags_NotFound)
		, m_dataScope(eDataScope_All)
		, m_streamTaskType(eStreamTaskTypeCount)
		, m_pMemoryBlock(nullptr)
		, m_pReadStream(nullptr)
		, m_pImplData(pImplData)
	{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
		m_timeCached.SetValue(0);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
	}

	CATLAudioFileEntry(CATLAudioFileEntry const&) = delete;
	CATLAudioFileEntry(CATLAudioFileEntry&&) = delete;
	CATLAudioFileEntry& operator=(CATLAudioFileEntry const&) = delete;
	CATLAudioFileEntry& operator=(CATLAudioFileEntry&&) = delete;

	CryFixedStringT<MaxFilePathLength> m_path;
	size_t                             m_size;
	size_t                             m_useCount;
	size_t                             m_memoryBlockAlignment;
	EnumFlagsType                      m_flags;
	EDataScope                         m_dataScope;
	EStreamTaskType                    m_streamTaskType;
	_smart_ptr<ICustomMemoryBlock>     m_pMemoryBlock;
	IReadStreamPtr                     m_pReadStream;
	Impl::IAudioFileEntry*             m_pImplData;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	CTimeValue m_timeCached;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
};

class CATLPreloadRequest final : public CATLEntity<PreloadRequestId>
{
public:
	typedef std::vector<FileEntryId> FileEntryIds;

	explicit CATLPreloadRequest(
	  PreloadRequestId const audioPreloadRequestId,
	  EDataScope const dataScope,
	  bool const bAutoLoad,
	  FileEntryIds const& fileEntryIds)
		: CATLEntity<PreloadRequestId>(audioPreloadRequestId, dataScope)
		, m_bAutoLoad(bAutoLoad)
		, m_fileEntryIds(fileEntryIds)
	{}

	bool const   m_bAutoLoad;
	FileEntryIds m_fileEntryIds;
};

//-------------------- ATLObject container typedefs --------------------------
typedef std::map<ControlId, CATLTrigger const*>              AudioTriggerLookup;
typedef std::map<ControlId, CParameter const*>               AudioParameterLookup;
typedef std::map<ControlId, CATLSwitch const*>               AudioSwitchLookup;
typedef std::map<PreloadRequestId, CATLPreloadRequest*>      AudioPreloadRequestLookup;
typedef std::map<EnvironmentId, CATLAudioEnvironment const*> AudioEnvironmentLookup;
} // namespace CryAudio
