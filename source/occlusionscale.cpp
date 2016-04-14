/*

Modo command to iterate selected meshes and rescale meshes that occlude (bounding boxes) more than a threshold

*/

/*
Include the required SDK header files.
*/

#include <occlusionscale.h>

/*
Define the server names.
*/

#define	OCCLUSIONSCALE_CMD	"items.os2"

/*
Define the functions and class for the items.occlusionScale command.
*/

class occlusionScale_cmd : public CLxBasicCommand
{
public:
	CLxUser_CommandService		cmd_svc;
	CLxUser_SelectionService	sel_svc;
	CLxUser_StdDialogService	dlg_svc;
	CLxUser_MessageService		msg_svc;

	int				basic_CmdFlags()						LXx_OVERRIDE;
	bool			basic_Enable(CLxUser_Message &msg)		LXx_OVERRIDE;
	void			cmd_Execute(unsigned int flags)			LXx_OVERRIDE;
	LxResult		cmd_Desc(const char **desc)				LXx_OVERRIDE;
	LxResult		cmd_ButtonName(const char **name)		LXx_OVERRIDE;
	LxResult		cmd_UserName(const char **name)			LXx_OVERRIDE;

	static LXtTagInfoDesc descInfo[];
	CLxUser_Log			  my_log;

	/*
	* Command argument indices.
	*/
	#define ARGi_OCCLUSION		 0
	#define ARGi_ITERATIONS		 1
	#define ARGi_SCALE			 2
	occlusionScale_cmd() 
	{ 
		dyna_Add("occlusion", LXsTYPE_FLOAT);
		dyna_Add("iterations", LXsTYPE_INTEGER);
		dyna_Add("scale", LXsTYPE_FLOAT);
		my_log.setByName("myLog"); 
	}
};

/*
Implementation of functions for the items.occlusionScale command.
*/

int occlusionScale_cmd::basic_CmdFlags()
{
	/*
	We're actually changing channels and item transforms here,
	so this needs to be an undoable command, which basically
	means that the user can undo and redo its actions.
	*/
	return LXfCMD_UNDO;
}

bool occlusionScale_cmd::basic_Enable(CLxUser_Message &msg)
{
	/*
	As the name suggests, this is where we tell modo if our command
	is enabled or not, so it can be disabled in the interface.
	We're going to simply check that at least two items are selected.
	*/
	if (sel_svc.Count(LXiSEL_ITEM) >= 2)
	{
		return true;
	}

	return false;
}

void occlusionScale_cmd::cmd_Execute(unsigned int flags)
{
	/*
	The function is called to execute the command. We're going to
	read the position, rotation and scale of the last selected item
	and use it to write the position, rotation and scale of the
	other selected items.
	*/

	CLxUser_ItemPacketTranslation	item_pkt;
	CLxUser_Message					&msg = basic_Message();

	unsigned			sel_count = 0;


	void *pkt = sel_svc.ByIndex(LXiSEL_ITEM, sel_count);

	item_pkt.autoInit();

	/*
	Get the current selected item count.
	*/
	sel_count = sel_svc.Count(LXiSEL_ITEM);

	/*
	Do another check to ensure that the user has two items selected.
	However, if they haven't, they shouldn't have made it this far.
	*/
	if (sel_count < 2)
	{
		msg.SetCode(LXe_FAILED);
		return;
	}

	double					maxOcclusion;
	int						iterations;
	double					scale;
	attr_GetFlt(ARGi_OCCLUSION, &maxOcclusion);
	attr_GetInt(ARGi_ITERATIONS, &iterations);
	attr_GetFlt(ARGi_SCALE, &scale);
	//float					occ;
	//LXtMatrix4				bb;

	OcclusionScale		occlusionScale(maxOcclusion, iterations, scale);

	// Test code
	/*my_log.Message(LXe_INFO, "bb a min: %f", occ);
	my_log.Message(LXe_INFO, "bb a min: %f %f %f %f", bb[0][0], bb[0][1], bb[0][2], bb[0][3]);
	my_log.Message(LXe_INFO, "bb b min: %f %f %f %f", bb[1][0], bb[1][1], bb[1][2], bb[1][3]);
	my_log.Message(LXe_INFO, "bb a max: %f %f %f %f", bb[2][0], bb[2][1], bb[2][2], bb[2][3]);
	my_log.Message(LXe_INFO, "bb b max: %f %f %f %f", bb[3][0], bb[3][1], bb[3][2], bb[3][3]);*/
}

LXtTagInfoDesc occlusionScale_cmd::descInfo[] =
{
	{ LXsSRV_LOGSUBSYSTEM, "myLog" },
	{ 0 }
};

/*
We don't actually need to implement the following functions. However
they provide some useful help and tips for the user in the modo commands
list. So it won't hurt.
*/

LxResult occlusionScale_cmd::cmd_Desc(const char **desc)
{
	/*
	This sets the command description in the commands list.
	*/
	desc[0] = "Iterate selected meshes and rescale meshes that occlude (bounding boxes) more than a threshold";
	return LXe_OK;
}

LxResult occlusionScale_cmd::cmd_ButtonName(const char **name)
{
	/*
	This sets the default button name if the command is added
	to a form.
	*/
	name[0] = "Scale Occluding";
	return LXe_OK;
}

LxResult occlusionScale_cmd::cmd_UserName(const char **name)
{
	/*
	This defines a user friendly name for the command.
	*/
	name[0] = "Scale Occluding";
	return LXe_OK;
}

/*
Initialize the servers.
*/

void initialize()
{
	CLxGenericPolymorph		*srv;

	srv = new CLxPolymorph						<occlusionScale_cmd>;

	srv->AddInterface(new CLxIfc_Command		<occlusionScale_cmd>);
	srv->AddInterface(new CLxIfc_Attributes		<occlusionScale_cmd>);
	srv->AddInterface(new CLxIfc_AttributesUI	<occlusionScale_cmd>);
	srv->AddInterface(new CLxIfc_StaticDesc		<occlusionScale_cmd>);

	thisModule.AddServer(OCCLUSIONSCALE_CMD, srv);
}
