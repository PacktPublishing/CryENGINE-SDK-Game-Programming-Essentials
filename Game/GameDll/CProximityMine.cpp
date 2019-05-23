#include "StdAfx.h"
#include "CProximityMine.h"
#include "IActorSystem.h"


CProximityMine::CProximityMine():
	m_pSelfTable(nullptr),
	m_Script_bActive(true),
	m_Script_bRigidBodyActive(true),
	m_Script_fMass(20.0f),
	m_Script_bIgnorePlayer(true),
	m_Script_fDetonationRadius(1.0f),
	m_Script_fExplosionStrength(200.0f),
	m_Script_object_Model("Game/Objects/Weapons/avmine/avmine.cgf")
{

}


CProximityMine::~CProximityMine()
{

}


/////////////////IGameObjectExtension///////////////////

void CProximityMine::GetMemoryUsage(ICrySizer *pSizer) const
{
	/// Letting The Profiler System Know The size (In Bytes) Of Our Class
	pSizer->Add(this);
}




bool CProximityMine::Init( IGameObject * pGameObject )
{
	/// Telling Our IGameObjectExtension (This Class) To Use The IGameObject That The IGameObjectSystem Created For Us
	SetGameObject(pGameObject);

	return true;
}




void CProximityMine::PostInit( IGameObject * pGameObject )
{
	IEntity *pEnt = GetEntity();

	if (pEnt)
	{
		// Grab Our Lua Script Values
		UpdateEntityTables();
		UpdateEntityProperties();

		// If Enabled Then Enable Updates
		if (m_Script_bActive)
			pGameObject->EnableUpdateSlot(this, 0);
		else
			pGameObject->DisableUpdateSlot(this, 0);

		// Load The Geometry That Was Specified In The Lua Script Properties
		pEnt->LoadGeometry(0, m_Script_object_Model, (const char*)NULL, IEntity::EEntityLoadFlags::EF_AUTO_PHYSICALIZE);

		// Physicalize Our Entity Using The Values From The Lua Script Properties
		SEntityPhysicalizeParams PhysParams;
		PhysParams.density = 0;

		//LB2KG Is Defined In "StdAfx.h" As #define LB2KG(Pounds) (double)Pounds / 2.2046
		PhysParams.mass = LB2KG(m_Script_fMass);
		PhysParams.nSlot = 0;
		// If RigidBody Is Active Then Simulate Using PE_RIGID(Dynamic) Else Simulate Using PE_STATIC(Static)
		if (m_Script_bRigidBodyActive)
			PhysParams.type = PE_RIGID;
		else
			PhysParams.type = PE_STATIC;

		pEnt->Physicalize(PhysParams);
	}
}




void CProximityMine::InitClient(int channelId)
{

}




void CProximityMine::PostInitClient(int channelId)
{

}




bool CProximityMine::ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{
	// Our Owning IGameObject May Have Been Deleted So Make Sure To Update Our IGameObjectExtension(This Class) To Relect That
	ResetGameObject();

	return true;
}




void CProximityMine::PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{

}




bool CProximityMine::GetEntityPoolSignature( TSerialize signature )
{
	// Return True To Notify The System That Our Serialization Signiture Is A-OKAY
	return true;
}




void CProximityMine::Release()
{
	// When The Entity System Has Deleted Our IEntity And IGameObject Then We Should Delete Our Extension
	delete this;
}




void CProximityMine::FullSerialize( TSerialize ser )
{

}




bool CProximityMine::NetSerialize( TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags )
{
	// Return True To Notify The System That Our Serialization Signiture Is A-OKAY
	return true;
}




void CProximityMine::PostSerialize()
{

}




void CProximityMine::SerializeSpawnInfo( TSerialize ser )
{

}




ISerializableInfoPtr CProximityMine::GetSpawnInfo()
{
	return nullptr;
}




void CProximityMine::Update( SEntityUpdateContext& ctx, int updateSlot )
{
	IEntity *pEnt = GetEntity();
	if (pEnt)
	{
		// Perform A Query To Find All Entities Withing A Certain Area Using The Specified Criteria
		SEntityProximityQuery Query;
		Query.box = AABB(pEnt->GetWorldPos(), m_Script_fDetonationRadius);
		Query.nEntityFlags = 0;
		Query.pEntityClass = nullptr;
		gEnv->pEntitySystem->QueryProximity(Query);

		IActor *pPlayerActor = gEnv->pGameFramework->GetClientActor();

		bool bExploded = false;

		for (int i = 0; i < Query.nCount; i++)
		{
			IEntity *pQueryEnt = Query.pEntities[i];
			IPhysicalEntity *pQueryPhysEnt = pQueryEnt->GetPhysics();

			// Only Process Entities That Are Not Other Proximity Mines ANd Are Not OurSelf (This Class Instance)
			if (pQueryPhysEnt && (pQueryEnt != pEnt) && (pQueryEnt->GetClass() != gEnv->pEntitySystem->GetClassRegistry()->FindClass("ProximityMine")))
			{
				// If We Should Ignore The Player Then Do Not Apply An Impulse To Him/Her. Else DO Apply An Impulse To Him/Her
				if (m_Script_bIgnorePlayer)
				{
					if (pQueryEnt != pPlayerActor->GetEntity())
					{
						// Add An Impulse To The Detonation Offending Entities
						pe_action_impulse Impulse;

						// Impulse Should Be Applied This Frame
						Impulse.iApplyTime = 0;

						// The Impulse Should Push Entities Away From Us(This Class Instance)
						Vec3 Dir = (pQueryEnt->GetWorldPos() - pEnt->GetWorldPos()).normalized();

						//Setting The Impulse Strength
						Dir.SetLength(m_Script_fExplosionStrength);

						Impulse.impulse = Dir;

						// Apply The Impulse
						pQueryPhysEnt->Action(&Impulse);

						bExploded = true;
					}
				}

				else
				{
					// Add An Impulse To The Detonation Offending Entities
					pe_action_impulse Impulse;

					// Impulse Should Be Applied This Frame
					Impulse.iApplyTime = 0;

					// The Impulse Should Push Entities Away From Us(This Class Instance)
					Vec3 Dir = (pQueryEnt->GetWorldPos() - pEnt->GetWorldPos()).normalized();

					//Setting The Impulse Strength
					Dir.SetLength(m_Script_fExplosionStrength);

					Impulse.impulse = Dir;

					// Apply The Impulse
					pQueryPhysEnt->Action(&Impulse);

					bExploded = true;
				}
			}

			IActor *pQueryActor = gEnv->pGameFramework->GetIActorSystem()->GetActor(pQueryEnt->GetId());
			if (pQueryActor)
			{
				// If We Should Ignore The Player Then Do Not Damage Him/Her. Else DO Damage Him/Her
				if (m_Script_bIgnorePlayer)
				{
					if (pQueryActor != pPlayerActor)
					{
						pQueryActor->SetHealth(-1);
						bExploded = true;
					}
				}
				else
				{
					pQueryActor->SetHealth(-1);
					bExploded = true;
				}
			}
		}

		// Once Exploded The Mine Should Be Deactivated
		if (bExploded)
			GetGameObject()->DisableUpdateSlot(this, 0);
	}
}




void CProximityMine::HandleEvent( const SGameObjectEvent& event )
{

}




void CProximityMine::ProcessEvent( SEntityEvent& event )
{
	EEntityEvent Event = event.event;

	switch (Event)
	{
	case ENTITY_EVENT_XFORM:
		break;
	case ENTITY_EVENT_TIMER:
		break;
	case ENTITY_EVENT_INIT:
		break;
	case ENTITY_EVENT_DONE:
		break;
	case ENTITY_EVENT_VISIBLITY:
		break;
	case ENTITY_EVENT_RESET:
		{
			// Entering Game Mode
			if (event.nParam[0] == 1)
			{

			}

			// Leaving Game Mode
			else
			{
				
			}
		}
		break;
	case ENTITY_EVENT_ATTACH:
		break;
	case ENTITY_EVENT_ATTACH_THIS:
		break;
	case ENTITY_EVENT_DETACH:
		break;
	case ENTITY_EVENT_DETACH_THIS:
		break;
	case ENTITY_EVENT_LINK:
		break;
	case ENTITY_EVENT_DELINK:
		break;
	case ENTITY_EVENT_HIDE:
		break;
	case ENTITY_EVENT_UNHIDE:
		break;
	case ENTITY_EVENT_ENABLE_PHYSICS:
		break;
	case ENTITY_EVENT_PHYSICS_CHANGE_STATE:
		break;
	case ENTITY_EVENT_SCRIPT_EVENT:
		break;
	case ENTITY_EVENT_ENTERAREA:
		break;
	case ENTITY_EVENT_LEAVEAREA:
		break;
	case ENTITY_EVENT_ENTERNEARAREA:
		break;
	case ENTITY_EVENT_LEAVENEARAREA:
		break;
	case ENTITY_EVENT_MOVEINSIDEAREA:
		break;
	case ENTITY_EVENT_MOVENEARAREA:
		break;
	case ENTITY_EVENT_CROSS_AREA:
		break;
	case ENTITY_EVENT_PHYS_POSTSTEP:
		break;
	case ENTITY_EVENT_PHYS_BREAK:
		break;
	case ENTITY_EVENT_AI_DONE:
		break;
	case ENTITY_EVENT_SOUND_DONE:
		break;
	case ENTITY_EVENT_NOT_SEEN_TIMEOUT:
		break;
	case ENTITY_EVENT_COLLISION:
		break;
	case ENTITY_EVENT_RENDER:
		break;
	case ENTITY_EVENT_PREPHYSICSUPDATE:
		break;
	case ENTITY_EVENT_LEVEL_LOADED:
		break;
	case ENTITY_EVENT_START_LEVEL:
		break;
	case ENTITY_EVENT_START_GAME:
		break;
	case ENTITY_EVENT_ENTER_SCRIPT_STATE:
		break;
	case ENTITY_EVENT_LEAVE_SCRIPT_STATE:
		break;
	case ENTITY_EVENT_PRE_SERIALIZE:
		break;
	case ENTITY_EVENT_POST_SERIALIZE:
		break;
	case ENTITY_EVENT_INVISIBLE:
		break;
	case ENTITY_EVENT_VISIBLE:
		break;
	case ENTITY_EVENT_MATERIAL:
		break;
	case ENTITY_EVENT_MATERIAL_LAYER:
		break;
	case ENTITY_EVENT_ONHIT:
		break;
	case ENTITY_EVENT_PICKUP:
		break;
	case ENTITY_EVENT_ANIM_EVENT:
		break;
	case ENTITY_EVENT_SCRIPT_REQUEST_COLLIDERMODE:
		break;
	case ENTITY_EVENT_ACTIVE_FLOW_NODE_OUTPUT:
		break;
	case ENTITY_EVENT_RELOAD_SCRIPT:
		break;
	case ENTITY_EVENT_MINE_PLACED:
		break;
	case ENTITY_EVENT_LAST:
		break;
	default:
		break;
	}
}




void CProximityMine::SetChannelId(uint16 id)
{

}




void CProximityMine::SetAuthority( bool auth )
{

}




const void *CProximityMine::GetRMIBase() const
{
	return CGameObjectExtensionHelper::GetRMIBase();
}




void CProximityMine::PostUpdate( float frameTime )
{

}




void CProximityMine::PostRemoteSpawn()
{

}


/////////////////CProximityMine///////////////////


bool CProximityMine::UpdateEntityProperties()
{
	if (!UpdateEntityTables())
		return false;

	bool bSuccess = true;

	IEntity *pEnt = GetEntity();
	if (!pEnt)
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The Entity's Script Properties Because The Entity Doesn't Exist Or Is NULL.");
		return bSuccess;
	}

	if (!m_pSelfTable)
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The Entity Table For Entity \"" + string(pEnt->GetName()) + "\" Because The Entity Table Doesn't Exist Or Is NULL.");
		return bSuccess;
	}

	if (!m_PropertiesTable.GetPtr())
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The \"Properties\" Table For Entity \"" + string(pEnt->GetName()) + "\" Because The \"Properties\" Table Doesn't Exist Or Is NULL.");
		return bSuccess;
	}

	if (!m_PhysicsTable.GetPtr())
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The \"Physics\" Table For Entity \"" + string(pEnt->GetName()) + "\" Because The \"Physics\" Table Doesn't Exist Or Is NULL.");
		return bSuccess;
	}

	if (!m_DetonationTable.GetPtr())
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The \"Detonation\" Table For Entity \"" + string(pEnt->GetName()) + "\" Because The \"Detonation\" Table Doesn't Exist Or Is NULL.");
		return bSuccess;
	}

	if (!m_VisualTable.GetPtr())
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The \"Visual\" Table For Entity \"" + string(pEnt->GetName()) + "\" Because The \"Visual\" Table Doesn't Exist Or Is NULL.");
		return bSuccess;
	}


	bSuccess = m_PropertiesTable->GetValue("bActive", m_Script_bActive);
	if (!bSuccess)
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The \"bActive\" Value From The \"Properties\" Table For Entity \"" + string(pEnt->GetName()) + "\" Because The \"bActive\" Value Doesn't Exist Or Another Error Occured.");

	bSuccess = m_PhysicsTable->GetValue("bRigidBodyActive", m_Script_bRigidBodyActive);
	if (!bSuccess)
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The \"bRigidBodyActive\" Value From The \"Physics\" Table For Entity \"" + string(pEnt->GetName()) + "\" Because The \"bRigidBodyActive\" Value Doesn't Exist Or Another Error Occured.");

	bSuccess = m_PhysicsTable->GetValue("fMass", m_Script_fMass);
	if (!bSuccess)
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The \"fMass\" Value From The \"Physics\" Table For Entity \"" + string(pEnt->GetName()) + "\" Because The \"fMass\" Value Doesn't Exist Or Another Error Occured.");

	bSuccess = m_DetonationTable->GetValue("bIgnorePlayer", m_Script_bIgnorePlayer);
	if (!bSuccess)
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The \"bIgnorePlayer\" Value From The \"Detonation\" Table For Entity \"" + string(pEnt->GetName()) + "\" Because The \"bIgnorePlayer\" Value Doesn't Exist Or Another Error Occured.");

	bSuccess = m_DetonationTable->GetValue("fDetonationRadius", m_Script_fDetonationRadius);
	if (!bSuccess)
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The \"fDetonationRadius\" Value From The \"Detonation\" Table For Entity \"" + string(pEnt->GetName()) + "\" Because The \"fDetonationRadius\" Value Doesn't Exist Or Another Error Occured.");

	bSuccess = m_DetonationTable->GetValue("fExplosionStrength", m_Script_fExplosionStrength);
	if (!bSuccess)
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The \"fExplosionStrength\" Value From The \"Detonation\" Table For Entity \"" + string(pEnt->GetName()) + "\" Because The \"fExplosionStrength\" Value Doesn't Exist Or Another Error Occured.");

	ScriptAnyValue object_ModelValue;
	bSuccess = m_VisualTable->GetValueAny("object_Model", object_ModelValue);
	m_Script_object_Model = object_ModelValue.str;
	if (!bSuccess)
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityProperties(): Failed To Update The \"object_Model\" Value From The \"Visual\" Table For Entity \"" + string(pEnt->GetName()) + "\" Because The \"object_Model\" Value Doesn't Exist Or Another Error Occured.");

	return bSuccess;
}




bool CProximityMine::UpdateEntityTables()
{

	bool bSuccess = true;

	IEntity *pEnt = GetEntity();
	if (!pEnt)
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityTables(): Failed To Update The Entity's Script Tables Because The Entity Doesn't Exist Or Is NULL.");
		return bSuccess;
	}

	m_pSelfTable = pEnt->GetScriptTable();
	if (!m_pSelfTable)
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityTables(): Failed To Load The Entity Table For Entity \"" + string(pEnt->GetName()) + "\".");
		return bSuccess;
	}

	m_pSelfTable->GetValue("Properties", m_PropertiesTable);
	if (!m_PropertiesTable.GetPtr())
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityTables(): Failed To Load The \"Properties\" Table For Entity \"" + string(pEnt->GetName()) + "\".");
		return bSuccess;
	}

	m_PropertiesTable->GetValue("Physics", m_PhysicsTable);
	if (!m_PhysicsTable.GetPtr())
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityTables(): Failed To Load The \"Physics\" Table For Entity \"" + string(pEnt->GetName()) + "\".");
		return bSuccess;
	}

	m_PropertiesTable->GetValue("Detonation", m_DetonationTable);
	if (!m_DetonationTable.GetPtr())
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityTables(): Failed To Load The \"Detonation\" Table For Entity \"" + string(pEnt->GetName()) + "\".");
		return bSuccess;
	}

	m_PropertiesTable->GetValue("Visual", m_VisualTable);
	if (!m_VisualTable.GetPtr())
	{
		bSuccess = false;
		gEnv->pLog->LogToConsole("CProximityMine::UpdateEntityTables(): Failed To Load The \"Visual\" Table For Entity \"" + string(pEnt->GetName()) + "\".");
		return bSuccess;
	}

	return bSuccess;
}