/*
 * MoleculeCBCudaRenderer.h
 *
 * Copyright (C) 2010 by Universitaet Stuttgart (VIS). Alle Rechte vorbehalten.
 */

#if (defined(WITH_CUDA) && (WITH_CUDA))

#ifndef MEGAMOL_MOLSURFREN_CONTOURBUILDUP_CUDA_H_INCLUDED
#define MEGAMOL_MOLSURFREN_CONTOURBUILDUP_CUDA_H_INCLUDED
#if (_MSC_VER > 1000)
#pragma once
#endif /* (_MSC_VER > 1000) */

#include "MolecularDataCall.h"
#include "param/ParamSlot.h"
#include "CallerSlot.h"
#include "CallFrame.h"
#include "view/Renderer3DModule.h"
#include "view/CallRender3D.h"
#include "vislib/SimpleFont.h"
#include <vislib/GLSLShader.h>
#include <vislib/Quaternion.h>
#include <vector>
#include <set>
#include <algorithm>
#include <list>
#include "vislib/FpsCounter.h"

#include "particles_kernel.cuh"
#include "vector_functions.h"
#include "cuda_runtime_api.h"
#include "cudpp/cudpp.h"

namespace megamol {
namespace protein {

	/**
	 * Molecular Surface Renderer class.
	 * Computes and renders the solvent excluded (Connolly) surface 
	 * using the Contour-Buildup Algorithm by Totrov & Abagyan.
	 */
	class MoleculeCBCudaRenderer : public megamol::core::view::Renderer3DModule
	{
	public:

		/**
		 * Answer the name of this module.
		 *
		 * @return The name of this module.
		 */
		static const char *ClassName(void)
		{
			return "MoleculeCBCudaRenderer";
		}

		/**
		 * Answer a human readable description of this module.
		 *
		 * @return A human readable description of this module.
		 */
		static const char *Description(void) 
		{
			return "Offers molecular surface renderings.";
		}

		/**
		 * Answers whether this module is available on the current system.
		 *
		 * @return 'true' if the module is available, 'false' otherwise.
		 */
		static bool IsAvailable(void) {
			//return true;
			return vislib::graphics::gl::GLSLShader::AreExtensionsAvailable();
		}
		
		/** ctor */
		MoleculeCBCudaRenderer(void);
		
		/** dtor */
		virtual ~MoleculeCBCudaRenderer(void);

	   /**********************************************************************
		 * 'get'-functions
		 **********************************************************************/
		
		/** Get probe radius */
		const float GetProbeRadius() const { return probeRadius; };

		/**********************************************************************
		 * 'set'-functions
		 **********************************************************************/

		/** Set probe radius */
		void SetProbeRadius( const float rad) { probeRadius = rad; };
		
	protected:
		
		/**
		 * Implementation of 'Create'.
		 *
		 * @return 'true' on success, 'false' otherwise.
		 */
		virtual bool create(void);
		
		/**
		 * Implementation of 'release'.
		 */
		virtual void release(void);

		/**
		 * Initialize CUDA
		 */
		bool initCuda( MolecularDataCall *mol, uint gridDim);

		/**
		 * Write atom positions and radii to an array for processing in CUDA
		 */
		void writeAtomPositions( const MolecularDataCall *mol );

    private:

        /**
         * The get capabilities callback. The module should set the members
         * of 'call' to tell the caller its capabilities.
         *
         * @param call The calling call.
         *
         * @return The return value of the function.
         */
        virtual bool GetCapabilities( megamol::core::Call& call);

        /**
         * The get extents callback. The module should set the members of
         * 'call' to tell the caller the extents of its data (bounding boxes
         * and times).
         *
         * @param call The calling call.
         *
         * @return The return value of the function.
         */
        virtual bool GetExtents( megamol::core::Call& call);

        /**
		 * Open GL Render call.
		 *
		 * @param call The calling call.
		 * @return The return value of the function.
		 */
		virtual bool Render( megamol::core::Call& call);

        /**
         * Update all parameter slots.
         *
         * @param mol   Pointer to the data call.
         */
        void UpdateParameters( const MolecularDataCall *mol);

		/**
		 * Deinitialises this renderer. This is only called if there was a 
		 * successful call to "initialise" before.
		 */
		virtual void deinitialise(void);
		
		/**********************************************************************
		 * variables
		 **********************************************************************/
		
		// caller slot
		megamol::core::CallerSlot molDataCallerSlot;
		
        // parameter slots
        megamol::core::param::ParamSlot probeRadiusParam;

		// camera information
		vislib::SmartPtr<vislib::graphics::CameraParameters> cameraInfo;

		// shader for the sphere raycasting
		vislib::graphics::gl::GLSLShader sphereShader;

		// the bounding box of the protein
		vislib::math::Cuboid<float> bBox;

		// radius of the probe atom
		float probeRadius;

		// max number of neighbors per atom
		const unsigned int atomNeighborCount;

		// CUDA Radix sort
        CUDPPHandle sortHandle;

		// params
		bool cudaInitalized;
		uint numAtoms;
		SimParams params;
		uint3 gridSize;
		uint numGridCells;

		// CPU data
		float* m_hPos;              // particle positions
		uint*  m_hNeighborCount;    // atom neighbor count
		uint*  m_hNeighbors;        // atom neighbor count
		float* m_hSmallCircles;     // small circles
		uint*  m_hParticleHash;
		uint*  m_hParticleIndex;
		uint*  m_hCellStart;
		uint*  m_hCellEnd;
        float* m_hArcs;

		// GPU data
		float* m_dPos;
		float* m_dSortedPos;
		uint*  m_dNeighborCount;
		uint*  m_dNeighbors;
		float* m_dSmallCircles;
        float* m_dArcs;

		// grid data for sorting method
		uint*  m_dGridParticleHash; // grid hash value for each particle
		uint*  m_dGridParticleIndex;// particle index for each particle
		uint*  m_dCellStart;        // index of start of each cell in sorted list
		uint*  m_dCellEnd;          // index of end of cell
		uint   gridSortBits;
		uint   m_colorVBO;          // vertex buffer object for colors
		float *m_cudaPosVBO;        // these are the CUDA deviceMem Pos
		float *m_cudaColorVBO;      // these are the CUDA deviceMem Color

	};

} /* end namespace protein */
} /* end namespace megamol */

#endif /* MEGAMOL_MOLSURFACERENDERERCONTOURBUILDUP_CUDA_H_INCLUDED */

#endif /* (defined(WITH_CUDA) && (WITH_CUDA)) */
