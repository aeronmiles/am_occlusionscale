/*
 * Plug-in component shader: cel shader
 *
 *   Copyright (c) 2008-2015 The Foundry Group LLC
 *   
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *   
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.   Except as contained
 *   in this notice, the name(s) of the above copyright holders shall not be
 *   used in advertising or otherwise to promote the sale, use or other dealings
 *   in this Software without prior written authorization.
 *   
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *   DEALINGS IN THE SOFTWARE.
 */

#include <lx_shade.hpp>
#include <lx_vector.hpp>
#include <lx_package.hpp>
#include <lx_action.hpp>
#include <lx_value.hpp>
#include <lx_log.hpp>
#include <lx_channelui.hpp>
#include <lx_item.hpp>
#include <lxcommand.h>
#include <lxidef.h>
#include <lx_raycast.hpp>
#include <lx_shdr.hpp>
#include <lx_tableau.hpp>

#include <math.h>
#include <string>



typedef struct st_LXpCelCM {
        float		diffLevel;
        float		specLevel;
        float		reflLevel;
} LXpCelCM;

class CelCMPacket : public CLxImpl_VectorPacket
{
    public:
        CelCMPacket () {}

        static LXtTagInfoDesc	descInfo[];

        unsigned int	vpkt_Size () LXx_OVERRIDE;
        const LXtGUID * vpkt_Interface (void) LXx_OVERRIDE;
        LxResult	vpkt_Initialize (void	*packet) LXx_OVERRIDE;
        LxResult	vpkt_Blend (void	*packet,void	*p0,void	*p1,float	t,int	mode) LXx_OVERRIDE;
};

#define SRVs_CEL_VPACKET		"cel.packet"
#define LXsP_SAMPLE_CEL			SRVs_CEL_VPACKET

LXtTagInfoDesc	 CelCMPacket::descInfo[] = {
        { LXsSRV_USERNAME,	"CelCM Packet" },
        { LXsSRV_LOGSUBSYSTEM,	"vector-packet"},
        { LXsVPK_CATEGORY,	LXsCATEGORY_SAMPLE},
        { 0 }
};

        unsigned int
CelCMPacket::vpkt_Size (void) 
{
        return	sizeof (LXpCelCM);
}
        
        const LXtGUID *
CelCMPacket::vpkt_Interface (void)
{
        return NULL;
}

        LxResult
CelCMPacket::vpkt_Initialize (
        void			*p) 
{
        LXpCelCM		*csp = (LXpCelCM *)p;

        csp->diffLevel = 0.5;
        csp->specLevel = 0.5;
        csp->reflLevel = 0.5;
        return LXe_OK;
}

        LxResult
CelCMPacket::vpkt_Blend (
        void			*p, 
        void			*p0, 
        void			*p1,
        float			 t,
        int			 mode)
{
/* This causes the layer opacity to blend FX packet parameters... */
        return LXe_OK;
}


/* --------------------------------- */
        
class CelCMPFX : public CLxImpl_PacketEffect
{
    public:
        CelCMPFX () {}

        static LXtTagInfoDesc	descInfo[];

        LxResult		pfx_Packet (const char **packet) LXx_OVERRIDE;
        unsigned int		pfx_Count (void) LXx_OVERRIDE;
        LxResult		pfx_ByIndex (int idx, const char **name, const char **typeName, int	*type) LXx_OVERRIDE;
        LxResult		pfx_Get (int idx,void *packet,float *val,void *item) LXx_OVERRIDE;
        LxResult		pfx_Set (int idx,void *packet,const float *val,void *item) LXx_OVERRIDE;
};

#define SRVs_CEL_PFX		"CelCMPFX"

#define SRVs_REFL_TFX		"celReflLevel"
#define SRVs_DIFF_TFX		"celDiffLevel"
#define SRVs_SPEC_TFX		"celSpecLevel"

LXtTagInfoDesc	 CelCMPFX::descInfo[] = {
        { LXsSRV_USERNAME,	"CelCM Packet FX" },
        { LXsSRV_LOGSUBSYSTEM,	"texture-effect"},
        { LXsTFX_CATEGORY,	LXsSHADE_SURFACE},
        { 0 }
};

        LxResult
CelCMPFX::pfx_Packet (const char	**packet) 
{
        packet[0] = SRVs_CEL_VPACKET;
        return LXe_OK;
}

        unsigned int
CelCMPFX::pfx_Count (void) 
{
        return	3;
}

        LxResult
CelCMPFX::pfx_ByIndex (int	id, const char **name, const char **typeName, int *type) 
{
        switch (id) {
                case 0:
                        name[0]     = SRVs_DIFF_TFX;
                        type[0]     = LXi_TFX_SCALAR | LXf_TFX_READ | LXf_TFX_WRITE;
                        typeName[0] = LXsTYPE_PERCENT;
                        break;
                case 1:
                        name[0]     = SRVs_SPEC_TFX;
                        type[0]     = LXi_TFX_SCALAR | LXf_TFX_READ | LXf_TFX_WRITE;
                        typeName[0] = LXsTYPE_PERCENT;
                        break;
                case 2:
                        name[0]     = SRVs_REFL_TFX;
                        type[0]     = LXi_TFX_SCALAR | LXf_TFX_READ | LXf_TFX_WRITE;
                        typeName[0] = LXsTYPE_PERCENT;
                        break;
        }
        return	LXe_OK;
}

        LxResult
CelCMPFX::pfx_Get (int  id, void *packet,float *val,void *item) 
{
        LXpCelCM	*csp = (LXpCelCM *) packet;
        switch (id) {
                case 0:
                        val[0] = csp->diffLevel;
                        break;
                case 1:
                        val[0] = csp->specLevel;
                        break;
                case 2:
                        val[0] = csp->reflLevel;
                        break;
        }
        return	LXe_OK;
}
        LxResult
CelCMPFX::pfx_Set (int  id, void *packet,const float *val,void *item)  
{
        LXpCelCM	*csp = (LXpCelCM *) packet;
        switch (id) {
                case 0:
                        csp->diffLevel = val[0];
                        break;
                case 1:
                        csp->specLevel = val[0];
                        break;
                case 2:
                        csp->reflLevel = val[0];
                        break;
        }
        return	LXe_OK;
}



/*
 * Cel material & shader
 * The cel shader is implemented as a custom material. Custom materials have the ability to set material attributes 
 * and to contribute to shading after the base shader has been evaluated
 */
class CelMaterial : public CLxImpl_CustomMaterial,
                public CLxImpl_ChannelUI
{
    public:
        CelMaterial () {}

        static LXtTagInfoDesc	descInfo[];

        int			cmt_Flags () LXx_OVERRIDE;
        LxResult		cmt_SetupChannels (ILxUnknownID addChan) LXx_OVERRIDE;
        LxResult		cmt_LinkChannels  (ILxUnknownID eval, ILxUnknownID item) LXx_OVERRIDE;
        LxResult		cmt_ReadChannels  (ILxUnknownID attr, void **ppvData) LXx_OVERRIDE;
        LxResult		cmt_CustomPacket  (const char	**) LXx_OVERRIDE;
        void			cmt_MaterialEvaluate      (
                                        ILxUnknownID		 etor,
                                        int			*idx,
                                        ILxUnknownID		 vector,
                                        void			*data) LXx_OVERRIDE;
        void			cmt_ShaderEvaluate      (
                                        ILxUnknownID		 vector,
                                        ILxUnknownID		 rayObj,
                                        LXpShadeComponents	*sCmp,
                                        LXpShadeOutput		*sOut,
                                        void			*data) LXx_OVERRIDE;
        void			cmt_Cleanup       (void *data) LXx_OVERRIDE;

        LxResult		cmt_SetOpaque       (int *opaque) LXx_OVERRIDE;
                
        LxResult		cui_UIHints	      (const char *channelName, ILxUnknownID hints) LXx_OVERRIDE;

        LXtItemType		MyType ();
        
  	LXtItemType		my_type; 	
        unsigned		pkt_offset;
        
        LXtSampleIndex          samp_idx[6];     // indices to each data channel in RendData
        int			cmt_IsSampleDriven (int *idx) LXx_OVERRIDE;
        LxResult		cmt_LinkSampleChannels (ILxUnknownID eval, ILxUnknownID item, int *idx) LXx_OVERRIDE;
        CLxUser_NodalService	nodalSvc;
        CLxUser_PacketService	pkt_service;

        class RendData {
                public:
                        int	      	diffBands;
                        int	      	specBands;
                        int	      	reflBands;
                        float		diffLevel, specLevel, reflLevel;
        };
};

#define SRVs_CEL_MATR			"celShader"
#define SRVs_CEL_MATR_ITEMTYPE		"material." SRVs_CEL_MATR

LXtTagInfoDesc	 CelMaterial::descInfo[] = {
        { LXsSRV_USERNAME,	"Cel Material" },
        { LXsSRV_LOGSUBSYSTEM,	"comp-shader"	},
        { 0 }
};

/*
 * clean up render data
 */
        void
CelMaterial::cmt_Cleanup (
        void			*data)
{
        RendData*       rd = (RendData*)data;

        delete rd;
}

/*
 * Setup channels for the item type.
 */
        int
CelMaterial::cmt_Flags ()
{
        return 0;
}
        
        LxResult
CelMaterial::cmt_SetupChannels (
        ILxUnknownID		 addChan)
{
        CLxUser_AddChannel	 ac (addChan);
     
        ac.NewChannel ("diffBands",            LXsTYPE_INTEGER);
        ac.SetDefault (0.0, 3);

        ac.NewChannel ("specBands",            LXsTYPE_INTEGER);
        ac.SetDefault (0.0, 0);

        ac.NewChannel ("reflBands",            LXsTYPE_INTEGER);
        ac.SetDefault (0.0, 0);

        ac.NewChannel ("diffLevel",        LXsTYPE_PERCENT);
        ac.SetDefault (0.50, 0);

        ac.NewChannel ("specLevel",        LXsTYPE_PERCENT);
        ac.SetDefault (0.50, 0);

        ac.NewChannel ("reflLevel",        LXsTYPE_PERCENT);
        ac.SetDefault (0.50, 0);

        return LXe_OK;
}

/*
 * Attach to channel evaluations.
 * This gets the indices for the channels in attributes.
 */
        LxResult
CelMaterial::cmt_LinkChannels (
        ILxUnknownID		 eval,
        ILxUnknownID		 item)
{
        CLxUser_Evaluation	 ev (eval);
        int			 i = 0;
        CLxUser_Item		 it (item);

        samp_idx[i++].chan = it.ChannelIndex ("diffBands");		
        samp_idx[i++].chan = it.ChannelIndex ("specBands");
        samp_idx[i++].chan = it.ChannelIndex ("reflBands");
        samp_idx[i++].chan = it.ChannelIndex ("diffLevel");
        samp_idx[i++].chan = it.ChannelIndex ("specLevel");
        samp_idx[i++].chan = it.ChannelIndex ("reflLevel");

        i = 0;
        samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"diffBands"		
        samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"specBands"
        samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"reflBands"
        samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"diffLevel"
        samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"specLevel"
        samp_idx[i].layer = ev.AddChan (item, samp_idx[i].chan); i++; //"reflLevel"

        pkt_offset  = pkt_service.GetOffset (LXsCATEGORY_SAMPLE, LXsP_SAMPLE_CEL);

        return LXe_OK;
}

                LxResult
CelMaterial::cmt_LinkSampleChannels (
        ILxUnknownID		 eval,
        ILxUnknownID		 item,
        int			*idx)
{
        CLxUser_Evaluation	 ev (eval);
        int i = 3; // skip band counts
        // the index of any channel that is not driven will be set to LXiNODAL_NOT_DRIVEN
        nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ); //"diffLevel
        nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ); //"specLevel
        nodalSvc.AddSampleChan (eval, item, samp_idx[i++].chan, idx, LXfECHAN_READ); //"reflLevel
        return LXe_OK;
}

                        int			
CelMaterial::cmt_IsSampleDriven (
        int *idx)  
{
         return nodalSvc.AnyDrivenChans (&idx[samp_idx[3].chan], 3); 
}

/*
 * Read channel values which may have changed.
 */
        LxResult
CelMaterial::cmt_ReadChannels (
        ILxUnknownID		 attr,
        void		       **ppvData)
{
        CLxUser_Attributes	 at (attr);
        RendData*                rd = new RendData;

        int			 i = 0;
   
        rd->diffBands           = at.Int (samp_idx[i++].layer);		
        rd->specBands           = at.Int (samp_idx[i++].layer);
        rd->reflBands           = at.Int (samp_idx[i++].layer);
        rd->diffLevel           = at.Float (samp_idx[i++].layer);
        rd->specLevel           = at.Float (samp_idx[i++].layer);
        rd->reflLevel           = at.Float (samp_idx[i++].layer);
        
        ppvData[0] = rd;
        return LXe_OK;
}

        static double
FVectorNormalize (
        LXtFVector		v) 
{
        float		m, p;
        m = LXx_VDOT (v, v);
        if(m<=0)
                return -1;
        m = sqrt (m);
        p = 1.0 / m;
        LXx_VSCL (v, p);
        return m;
}

        static void
FVectorQuantize (
        LXtFVector		vec,
        int			bnd,
        float			level) 
{
        float			vec0[3], len;
        int			i;
        
        LXx_VCPY (vec0, vec);
        len = FVectorNormalize (vec0);
        
        for (i=0;i<3;i++)
                vec[i] = vec0[i]*(floor(len*(bnd-1)) + level)/(float)bnd;	
}

/*
 * Since the cel shader is modifying the results of another shader,
 * it cannot be opaque.
 */

        LxResult
CelMaterial::cmt_SetOpaque (
        int			*opaque)
{
        *opaque = 0;

        return LXe_OK;
}

                LxResult
CelMaterial::cmt_CustomPacket (
        const char		**packet)
{
        packet[0] = LXsP_SAMPLE_CEL;
        return LXe_OK;
}

/*
 * Set custom material values at a spot
 */
        void
CelMaterial::cmt_MaterialEvaluate (
        ILxUnknownID		 etor,
        int			*idx,
        ILxUnknownID            vector,
        void			*data)
{
        LXpCelCM		*celPkt = (LXpCelCM*) pkt_service.FastPacket (vector, pkt_offset);
        RendData*       	 rd = (RendData*)data;

        celPkt->diffLevel = nodalSvc.GetFloat (etor, idx, samp_idx[3].chan, rd->diffLevel );
        celPkt->specLevel = nodalSvc.GetFloat (etor, idx, samp_idx[4].chan, rd->specLevel );
        celPkt->reflLevel = nodalSvc.GetFloat (etor, idx, samp_idx[5].chan, rd->reflLevel );
}

/*
 * Evaluate the color at a spot.
 */
        void
CelMaterial::cmt_ShaderEvaluate (
        ILxUnknownID            vector,
        ILxUnknownID		rayObj,
        LXpShadeComponents     *sCmp,
        LXpShadeOutput         *sOut,
        void                    *data)
{
        LXpCelCM		*celPkt = (LXpCelCM*) pkt_service.FastPacket (vector, pkt_offset);
        RendData		*rd   = (RendData *) data;

        /*
         * Quantize each component with the bands and level settings
         */

        if (rd->diffBands) 
                FVectorQuantize (sCmp->diff, rd->diffBands, celPkt->diffLevel);
        if (rd->specBands) 
                FVectorQuantize (sCmp->spec, rd->specBands, celPkt->specLevel);
        if (rd->reflBands) 
                FVectorQuantize (sCmp->refl, rd->reflBands, celPkt->reflLevel);

        // Update final output color		
        for (int i = 0; i < 3; i++) 
                sOut->color[i] = sCmp->diff[i] + sCmp->spec[i] + sCmp->refl[i] + sCmp->tran[i] + sCmp->subs[i] + sCmp->lumi[i];
}

          LxResult
CelMaterial::cui_UIHints (
        const char		*channelName,
        ILxUnknownID		 hints)
{
        if( !strcmp(channelName, "diffLevel")  || !strcmp(channelName, "diffBands") )
        {
                CLxUser_UIHints		 ui (hints);
                ui.ChannelFlags (LXfUIHINTCHAN_SUGGESTED);
                return LXe_OK;
        }
        
        return LXe_NOTIMPL;
}


/*
 * Utility to get the type code for this item type, as needed.
 */
        LXtItemType
CelMaterial::MyType ()
{
        if (my_type != LXiTYPE_NONE)
                return my_type;

        CLxUser_SceneService	 svc;

        my_type = svc.ItemType (SRVs_CEL_MATR_ITEMTYPE);
        return my_type;
}

        void
initialize ()
{
    CLxGenericPolymorph*    srv1 = new CLxPolymorph<CelMaterial>;

    srv1->AddInterface (new CLxIfc_CustomMaterial<CelMaterial>);
    srv1->AddInterface (new CLxIfc_StaticDesc<CelMaterial>);
    lx::AddServer (SRVs_CEL_MATR, srv1);

    CLxGenericPolymorph*    srv2 = new CLxPolymorph<CelCMPacket>;
    CLxGenericPolymorph*    srv3 = new CLxPolymorph<CelCMPFX>;
    srv2->AddInterface (new CLxIfc_VectorPacket<CelCMPacket>);
    srv2->AddInterface (new CLxIfc_StaticDesc<CelCMPacket>);
    lx::AddServer (SRVs_CEL_VPACKET, srv2);

    srv3->AddInterface (new CLxIfc_PacketEffect<CelCMPFX>);
    srv3->AddInterface (new CLxIfc_StaticDesc<CelCMPFX>);
    lx::AddServer (SRVs_CEL_PFX, srv3);

}

