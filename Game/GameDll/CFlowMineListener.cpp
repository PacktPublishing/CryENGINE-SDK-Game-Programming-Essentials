#include "StdAfx.h"
#include "Nodes\G2FlowBaseNode.h"




class CFlowMineListener : public CFlowBaseNode<eNCT_Instanced>, public IEntityEventListener
{
public:
	CFlowMineListener(SActivationInfo *pActInfo):
		pTargetEnt(nullptr)
	{
		
	}

	~CFlowMineListener()
	{
		if (pTargetEnt)
			gEnv->pEntitySystem->RemoveEntityEventListener(pTargetEnt->GetId(), ENTITY_EVENT_MINE_PLACED, this);

	}

	virtual IFlowNodePtr Clone( SActivationInfo *pActInfo)
	{
		return new CFlowMineListener(pActInfo);
	}

	virtual void GetConfiguration( SFlowNodeConfig& Config)
	{

		static const SOutputPortConfig Outputs[] = 
		{
			OutputPortConfig_Void("MinePlaced", _HELP("Triggered When A Mine Is Placed")),
			OutputPortConfig<EntityId>("Placer", _HELP("The EntityID Of The Entity That Placed The Mine")),
			OutputPortConfig<EntityId>("Mine", _HELP("The EntityID Of The Mine That Was Placed"))
		};

		Config.pOutputPorts = Outputs;
		Config.SetCategory(EFLN_APPROVED);
		Config.sDescription = _HELP("Listens For Mines To Be Placed");
		Config.nFlags |= EFLN_TARGET_ENTITY;
	}

	enum EOP
	{
		EOP_MINEPLACED = 0,
		EOP_PLACER,
		EOP_MINE
	};

	virtual void Serialize( SActivationInfo *, TSerialize ser )
	{

	}

	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo)
	{
		switch (event)
		{
		case IFlowNode::eFE_Update:
			break;
		case IFlowNode::eFE_Activate:
			break;
		case IFlowNode::eFE_FinalActivate:
			break;
		case IFlowNode::eFE_PrecacheResources:
			break;
		case IFlowNode::eFE_Initialize:
			{
				ActInfo = *pActInfo;
			}
			break;
		case IFlowNode::eFE_FinalInitialize:
			break;
		case IFlowNode::eFE_SetEntityId:
			{
				if (pTargetEnt)
					gEnv->pEntitySystem->RemoveEntityEventListener(pTargetEnt->GetId(), ENTITY_EVENT_MINE_PLACED, this);

				if (pActInfo->pEntity)
					gEnv->pEntitySystem->AddEntityEventListener(pActInfo->pEntity->GetId(), ENTITY_EVENT_MINE_PLACED, this);

				pTargetEnt = pActInfo->pEntity;
			}
			break;
		case IFlowNode::eFE_Suspend:
			break;
		case IFlowNode::eFE_Resume:
			break;
		case IFlowNode::eFE_ConnectInputPort:
			break;
		case IFlowNode::eFE_DisconnectInputPort:
			break;
		case IFlowNode::eFE_ConnectOutputPort:
			break;
		case IFlowNode::eFE_DisconnectOutputPort:
			break;
		case IFlowNode::eFE_DontDoAnythingWithThisPlease:
			break;
		default:
			break;
		}
	}

	virtual void GetMemoryUsage(ICrySizer *s) const
	{
		s->Add(this);
	}

	virtual void OnEntityEvent( IEntity *pEntity, SEntityEvent &event )
	{
		EEntityEvent Event = event.event;

		if (event.event == ENTITY_EVENT_MINE_PLACED)
		{
			if (pTargetEnt)
			{
				ActivateOutput(&ActInfo, EOP_PLACER, pTargetEnt->GetId());
				ActivateOutput(&ActInfo, EOP_MINE, event.nParam[0]);
				ActivateOutput(&ActInfo, EOP_MINEPLACED, NULL); 
			}
		}
	}

private:

	SActivationInfo ActInfo;
	IEntity *pTargetEnt;
};


REGISTER_FLOW_NODE("Packt:MineLister", CFlowMineListener);