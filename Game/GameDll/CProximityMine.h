#pragma once
#include "IGameObject.h"

class CProximityMine : public CGameObjectExtensionHelper<CProximityMine,IGameObjectExtension>
{
public:
	CProximityMine();
	virtual ~CProximityMine();

	/////////////////IGameObjectExtension///////////////////

	virtual void GetMemoryUsage(ICrySizer *pSizer) const;
	virtual bool Init( IGameObject * pGameObject );
	virtual void PostInit( IGameObject * pGameObject );
	virtual void InitClient(int channelId);
	virtual void PostInitClient(int channelId);
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize ser );
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags );
	virtual void PostSerialize();
	virtual void SerializeSpawnInfo( TSerialize ser );
	virtual ISerializableInfoPtr GetSpawnInfo();
	virtual void Update( SEntityUpdateContext& ctx, int updateSlot );
	virtual void HandleEvent( const SGameObjectEvent& event );
	virtual void ProcessEvent( SEntityEvent& event );	
	virtual void SetChannelId(uint16 id);
	virtual void SetAuthority( bool auth );
	virtual const void * GetRMIBase() const;
	virtual void PostUpdate( float frameTime );
	virtual void PostRemoteSpawn();

	/////////////////CProximityMine///////////////////

	/// <summary>
	/// Updates The C++ Version Of The Entity's Properties By Reading In The Current Values Of The Entity's Lua Script
	/// </summary>
	/// <returns>True If Updating Was Successful, False Otherwise</returns>
	bool UpdateEntityProperties();

	/// <summary>
	/// Updates The C++ Version Of The Entity's Lua Script Tables
	/// </summary>
	/// <returns>True If Updating Was Successful, False Otherwise</returns>
	bool UpdateEntityTables();

private:

	/// Script Tables
	IScriptTable *m_pSelfTable;
	SmartScriptTable m_PropertiesTable;
	SmartScriptTable m_PhysicsTable;
	SmartScriptTable m_DetonationTable;
	SmartScriptTable m_VisualTable;

	///Entity Properties
	bool   m_Script_bActive;
	bool   m_Script_bRigidBodyActive;
	float  m_Script_fMass;
	bool   m_Script_bIgnorePlayer;
	float  m_Script_fDetonationRadius;
	float  m_Script_fExplosionStrength;
	string m_Script_object_Model;
};

