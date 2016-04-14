#include <lxidef.h>
#include <lx_plugin.hpp>
#include <lxu_command.hpp>
#include <lx_command.hpp>
#include <lx_locator.hpp>
#include <lx_stdDialog.hpp>
#include <lxu_math.hpp>
#include <lxvmath.h>
#include <lx_log.hpp>

class OcclusionScale : public CLxImpl_AbstractVisitor
{
private:
	CLxUser_SelectionService			sel_svc;	
	int									sel_count = sel_svc.Count(LXiSEL_ITEM);
	double								current_time = sel_svc.GetTime();
	CLxUser_ItemPacketTranslation		item_pkt;
	void								*pkt_a;
	void								*pkt_b;

	CLxUser_Item						item_loc;	
	CLxUser_Locator						locator_loc;
	CLxUser_Point						point;
	CLxUser_Matrix			  			world_matrix;
	LXtMatrix4							world_matrix4;

	LXtMatrix4							scale_matrix4;
	CLxUser_Matrix						scale_matrix;
	LXtVector							bb_min_a;
	LXtVector							bb_max_a;
	LXtVector							bb_min_b;
	LXtVector							bb_max_b;

	CLxUser_Scene						scene;
	unsigned							chan_index;	
	CLxUser_ChannelRead					chan_read;
	CLxUser_ChannelWrite				chan_write;	
	CLxUser_Mesh						mesh;
	LXtBBox								bb;

	float						 		occlusion = 0.0f;

public:
	OcclusionScale(double &max_occlusion, int &iterations, double &scale, float &occ, LXtMatrix4 &bb)
	{		
		item_pkt.autoInit();		

		//pkt_a = sel_svc.ByIndex(LXiSEL_ITEM, 0);
		//item_pkt.Item(pkt_a, item_loc);
		unsigned ind = 0;
		CheckOcclusion(ind, occlusion, bb);
		//occ = occlusion;
		//for (unsigned iter = 0; iter < iterations; iter++)
		//{
		//	for (unsigned ind = 0; ind < sel_count; ind++)
		//	{
		//		CheckOcclusion(ind, occlusion);
		//		if (occlusion > max_occlusion)
		//		{
		//			Rescale(ind, scale);
		//		}
		//	}
		//}
	}

	LxResult Rescale(unsigned &ind, double &scale)
	{
		pkt_a = sel_svc.ByIndex(LXiSEL_ITEM, ind);
		item_pkt.Item(pkt_a, item_loc);

		item_loc.GetContext(scene);
		scene.GetChannels(chan_read, current_time);
		scene.SetChannels(chan_write, LXs_ACTIONLAYER_EDIT);

		chan_read.Object(item_loc, LXsICHAN_XFRMCORE_LOCALMATRIX, scale_matrix);

		scale_matrix.Get4(scale_matrix4);
		scale_matrix4[0][0] *= scale;
		scale_matrix4[1][1] *= scale;
		scale_matrix4[2][2] *= scale;

		item_pkt.Item(pkt_a, locator_loc);
		if (locator_loc.SetScale(chan_read, chan_write, scale_matrix4, LXiLOCATOR_LOCAL, 0) != LXe_OK)
		{
			/*
			For whatever reason, we weren't able to set the transforms.
			I'm unsure of how best to handle this, for now, we'll return
			an error and stop the command. But that's not ideal.
			*/
			//msg.SetCode(LXe_FAILED);
		}
		return LXe_OK;
	}
	
	LxResult CheckOcclusion(unsigned &ind, float &occlusion, LXtMatrix4 &bb)
	{
		pkt_a = sel_svc.ByIndex(LXiSEL_ITEM, ind);
		item_pkt.Item(pkt_a, item_loc);
		bbXformed(bb_min_a, bb_max_a);

		occlusion = 0.0f;
		for (int i = 0; i < sel_count; i++)
		{
			pkt_b = sel_svc.ByIndex(LXiSEL_ITEM, i);
			item_pkt.Item(pkt_b, item_loc);
			bbXformed(bb_min_b, bb_max_b);

			if (bb_min_b[0] > bb_min_a[0]
				&& bb_min_b[1] > bb_min_a[1]
				&& bb_min_b[2] > bb_min_a[2]
				&& bb_max_b[0] < bb_max_a[0]
				&& bb_max_b[1] < bb_max_a[1]
				&& bb_max_b[2] < bb_max_a[2]
				)
			{
				occlusion += 1.0f;
			}
		}

		bb[0][0] = bb_min_a[0];
		bb[0][1] = bb_min_a[1];
		bb[0][2] = bb_min_a[2];
		bb[1][0] = bb_min_b[0];
		bb[1][1] = bb_min_b[1];
		bb[1][2] = bb_min_b[2];
		bb[2][0] = bb_max_a[0];
		bb[2][1] = bb_max_a[1];
		bb[2][2] = bb_max_a[2];
		bb[3][0] = bb_max_b[0];
		bb[3][1] = bb_max_b[1];
		bb[3][2] = bb_max_b[2];

		occlusion /= sel_count;

		return LXe_OK;
	}

	LxResult bbXformed(LXtVector &bb_min, LXtVector &bb_max)
	{
		///* // Get mesh from item mesh channel https://community.thefoundry.co.uk/discussion/topic.aspx?mode=Post&f=37&t=79387&p=713613
		//scene.SetChannels(chan_write, LXs_ACTIONLAYER_EDIT);
		if (LXx_OK(item_loc.ChannelLookup(LXsICHAN_MESH_MESH, &chan_index)))
		{
			item_loc.GetContext(scene);
			scene.GetChannels(chan_read, LXs_ACTIONLAYER_EDIT);
			if (chan_read.Object(item_loc, chan_index, mesh))
			{
				mesh.BoundingBox(LXiMARK_ANY, &bb);

				chan_read.Object(item_loc, LXsICHAN_XFRMCORE_WORLDMATRIX, world_matrix);
				world_matrix.Get4(world_matrix4);

				lx::Matrix4Multiply(bb_min, world_matrix4, bb.min);
				lx::Matrix4Multiply(bb_max, world_matrix4, bb.max);
			}
		}
		return LXe_OK;
	}

	LxResult Evaluate()    LXx_OVERRIDE
	{
		return LXe_OK;
	}
};


//for (unsigned j = 0; j < 4; j++) {
//	for (unsigned k = 0; k < 4; k++) {
//		bb[j][k] = scale_matrix4[j][k];
//	}
//}

//bb[0][0] = bb_min_a[0];
//bb[0][1] = bb_min_a[1];
//bb[0][2] = bb_min_a[2];
//bb[1][0] = bb_min_b[0];
//bb[1][1] = bb_min_b[1];
//bb[1][2] = bb_min_b[2];
//bb[2][0] = bb_max_a[0];
//bb[2][1] = bb_max_a[1];
//bb[2][2] = bb_max_a[2];
//bb[3][0] = bb_max_b[0];
//bb[3][1] = bb_max_b[1];
//bb[3][2] = bb_max_b[2];