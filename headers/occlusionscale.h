#include <lxidef.h>
#include <lxu_command.hpp>

//occlusionscale.cpp
#include <lx_plugin.hpp>
#include <lx_stdDialog.hpp>
#include <lx_log.hpp>

//occlusionscale.h
#include <lx_locator.hpp>
#include <lxu_math.hpp>
#include <lx_layer.hpp>

class OcclusionScale : public CLxImpl_AbstractVisitor
{
private:
	CLxUser_SelectionService			sel_svc;	
	CLxUser_LayerService				layer_svc;
	CLxUser_LayerScan 					layer_scn;
	unsigned							sel_count = sel_svc.Count(LXiSEL_ITEM);
	double								current_time = sel_svc.GetTime();
	CLxUser_ItemPacketTranslation		item_pkt;
	void								*pkt;

	CLxUser_Item						item_loc;	
	CLxUser_Locator						locator_loc;
	CLxUser_Matrix			  			local_matrix;
	LXtMatrix4							local_matrix4;

	LXtMatrix4							scale_matrix4;
	CLxUser_Matrix						scale_matrix;

	CLxUser_Scene						scene;
	unsigned							chan_index;	
	CLxUser_ChannelRead					chan_read;
	CLxUser_ChannelWrite				chan_write;	
	CLxUser_Mesh						mesh;
	LXtBBox								bb_a;
	LXtBBox								bb_b;

	float						 		occlusion = 0.0f;

public:
	OcclusionScale(double &max_occlusion, int &iterations, double &scale)
	{		
		item_pkt.autoInit();	
		layer_svc.BeginScan(LXf_LAYERSCAN_ACTIVE, layer_scn);

		for (int iter = 0; iter < iterations; iter++)
		{
			for (unsigned ind = 0; ind < sel_count; ind++)	
			{
				CheckOcclusion(ind, occlusion);				
				if (occlusion > max_occlusion)
				{		
					Rescale(ind, scale);
				}
			}
		}
	}

	LxResult Rescale(unsigned &ind, double &scale)
	{
		pkt = sel_svc.ByIndex(LXiSEL_ITEM, ind);
		item_pkt.Item(pkt, item_loc);

		item_loc.GetContext(scene);
		scene.GetChannels(chan_read, current_time);
		scene.SetChannels(chan_write, LXs_ACTIONLAYER_EDIT);

		chan_read.Object(item_loc, LXsICHAN_XFRMCORE_LOCALMATRIX, scale_matrix);
		scale_matrix.Get4(scale_matrix4);

		scale_matrix4[0][0] *= scale;
		scale_matrix4[1][1] *= scale;
		scale_matrix4[2][2] *= scale;

		item_pkt.Item(pkt, locator_loc);
		locator_loc.SetScale(chan_read, chan_write, scale_matrix4, LXiLOCATOR_LOCAL, 0);

		return LXe_OK;
	}
	
	LxResult CheckOcclusion(unsigned &ind, float &occlusion)
	{
		bbXformed(ind, bb_a);

		occlusion = 0.0f;
		for (unsigned i = 0; i < sel_count; i++)
		{
			bbXformed(i, bb_b);
			if (bb_b.min[0] > bb_a.min[0]
				&& bb_b.min[1] > bb_a.min[1]
				&& bb_b.min[2] > bb_a.min[2]
				&& bb_b.max[0] < bb_a.max[0]
				&& bb_b.max[1] < bb_a.max[1]
				&& bb_b.max[2] < bb_a.max[2]
				)
			{
				occlusion += 1.0f;
			}
		}

		occlusion /= sel_count;

		return LXe_OK;
	}

	LxResult bbXformed(unsigned &ind, LXtBBox &bb)
	{
		pkt = sel_svc.ByIndex(LXiSEL_ITEM, ind);
		item_pkt.Item(pkt, item_loc);
		item_loc.GetContext(scene);
		scene.GetChannels(chan_read, current_time);

		if (item_loc.ChannelLookup(LXsICHAN_MESH_MESH, &chan_index))
		{
			layer_scn.BaseMeshByIndex(ind, mesh);
			mesh.BoundingBox(LXiMARK_ANY, &bb);

			chan_read.Object(item_loc, LXsICHAN_XFRMCORE_LOCALMATRIX, local_matrix);
			local_matrix.Get4(local_matrix4);

			lx::Matrix4Multiply(bb.min, local_matrix4, bb.min);
			lx::Matrix4Multiply(bb.max, local_matrix4, bb.max);

			return LXe_OK;
		}
		else
		{
			return LXe_FAILED;
		}
	}

	LxResult Evaluate()    LXx_OVERRIDE
	{
		return LXe_OK;
	}
};
