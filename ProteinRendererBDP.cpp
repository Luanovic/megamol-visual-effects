/*
 * ProteinRendererBDP.cpp
 *
 * Copyright (C) 2009 by Universitaet Stuttgart (VIS). Alle Rechte vorbehalten.
*/


#include "stdafx.h"

#define _USE_MATH_DEFINES 1

#include "ProteinRendererBDP.h"

#include "CoreInstance.h"
#include "param/EnumParam.h"
#include "param/BoolParam.h"
#include "param/FloatParam.h"
#include "param/IntParam.h"
#include "vislib/assert.h"
#include "vislib/File.h"
#include "vislib/String.h"
#include "vislib/OutOfRangeException.h"
#include "vislib/Trace.h"
#include "vislib/ShaderSource.h"
#include "vislib/AbstractOpenGLShader.h"
#include "glh/glh_genext.h"
#include "glh/glh_extensions.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <ctime>
#include <fstream>
#include <math.h>

using namespace megamol::core;
using namespace megamol::protein;

/*
 * ProteinRendererBDP::ProteinRendererBDP
 */
ProteinRendererBDP::ProteinRendererBDP( void ) : Renderer3DModule (),
    protDataCallerSlot( "getData", "Connects the protein SES rendering with protein data storage" ),
    postprocessingParam( "postProcessingMode", "Enable Postprocessing Mode: "),
    rendermodeParam( "renderingMode", "Choose Render Mode: "),
    coloringmodeParam( "coloringMode", "Choose Coloring Mode: "),
    silhouettecolorParam( "silhouetteColor", "Silhouette Color: "),
    sigmaParam( "SSAOsigma", "Sigma value for SSAO: " ),
    lambdaParam( "SSAOlambda", "Lambda value for SSAO: "),
    minvaluecolorParam( "minValueColor", "Color for minimum value: "),
    maxvaluecolorParam( "maxValueColor", "Color for maximum value: "),
    meanvaluecolorParam( "meanValueColor", "Color for mean value: "),
    debugParam( "drawRS", "Draw the Reduced Surface: "),
    drawSESParam( "drawSES", "Draw the SES: "),
    drawSASParam( "drawSAS", "Draw the SAS: "),
    fogstartParam( "fogStart", "Fog Start: ")
{    
    this->protDataCallerSlot.SetCompatibleCall<CallProteinDataDescription>();
    this->MakeSlotAvailable ( &this->protDataCallerSlot );

    // set epsilon value for float-comparison
    this->epsilon = vislib::math::FLOAT_EPSILON;
    // set probe radius
    this->probeRadius = 1.4f;
    // set transparency
    this->transparency = 0.5f;

    // ----- en-/disable postprocessing -----
    this->postprocessing = NONE;
    //this->postprocessing = AMBIENT_OCCLUSION;
    //this->postprocessing = SILHOUETTE;
    //this->postprocessing = TRANSPARENCY;
    param::EnumParam *ppm = new param::EnumParam ( int ( this->postprocessing ) );
    ppm->SetTypePair( NONE, "None");
    ppm->SetTypePair( AMBIENT_OCCLUSION, "Screen Space Ambient Occlusion");
    ppm->SetTypePair( SILHOUETTE, "Silhouette");
    ppm->SetTypePair( TRANSPARENCY, "Transparency");
    this->postprocessingParam << ppm;

    // ----- choose current render mode -----
    this->currentRendermode = GPU_RAYCASTING;
    //this->currentRendermode = POLYGONAL;
    //this->currentRendermode = POLYGONAL_GPU;
    //this->currentRendermode = GPU_SIMPLIFIED;
    param::EnumParam *rm = new param::EnumParam ( int ( this->currentRendermode ) );
    rm->SetTypePair( GPU_RAYCASTING, "GPU Ray Casting");
    rm->SetTypePair( GPU_SIMPLIFIED, "GPU Ray Casting (simplified)");
    this->rendermodeParam << rm;

    // ----- set the default color for the silhouette -----
    this->SetSilhouetteColor( 1.0f, 1.0f, 1.0f);
    param::IntParam *sc = new param::IntParam( this->codedSilhouetteColor, 0, 255255255 );
    this->silhouettecolorParam << sc;

    // ----- set sigma for screen space ambient occlusion (SSAO) -----
    this->sigma = 5.0f;
    param::FloatParam *ssaos = new param::FloatParam( this->sigma );
    this->sigmaParam << ssaos;

    // ----- set lambda for screen space ambient occlusion (SSAO) -----
    this->lambda = 10.0f;
    param::FloatParam *ssaol = new param::FloatParam( this->lambda );
    this->lambdaParam << ssaol;

    // ----- set start value for fogging -----
    this->fogStart = 0.5f;
    param::FloatParam *fs = new param::FloatParam( this->fogStart, 0.0f );
    this->fogstartParam << fs;

    // ----- choose current coloring mode -----
    this->currentColoringMode = ELEMENT;
    //this->currentColoringMode = AMINOACID;
    //this->currentColoringMode = STRUCTURE;
    //this->currentColoringMode = CHAIN_ID;
    //this->currentColoringMode = VALUE;
    param::EnumParam *cm = new param::EnumParam ( int ( this->currentColoringMode ) );
    cm->SetTypePair( ELEMENT, "Element");
    cm->SetTypePair( AMINOACID, "AminoAcid");
    cm->SetTypePair( STRUCTURE, "SecondaryStructure");
    cm->SetTypePair( CHAIN_ID, "ChainID");
    cm->SetTypePair( VALUE, "Value");
    cm->SetTypePair( RAINBOW, "Rainbow");
    cm->SetTypePair( CHARGE, "Charge");
    this->coloringmodeParam << cm;

    // ----- set the default color for minimum value -----
    this->SetMinValueColor( 0.0f, 0.0f, 1.0f);
    param::IntParam *minc = new param::IntParam( this->codedMinValueColor, 0, 255255255 );
    this->minvaluecolorParam << minc;
    // ----- set the default color for mean value -----
    this->SetMeanValueColor( 1.0f, 1.0f, 1.0f);
    param::IntParam *meanc = new param::IntParam( this->codedMeanValueColor, 0, 255255255 );
    this->meanvaluecolorParam << meanc;
    // ----- set the default color for maximum value -----
    this->SetMaxValueColor( 1.0f, 0.0f, 0.0f);
    param::IntParam *maxc = new param::IntParam( this->codedMaxValueColor, 0, 255255255 );
    this->maxvaluecolorParam << maxc;

    // ----- draw RS param -----
    this->drawRS = false;
    param::BoolParam *bpm = new param::BoolParam( this->drawRS );
    this->debugParam << bpm;

    // ----- draw SES param -----
    this->drawSES = true;
    param::BoolParam *sespm = new param::BoolParam( this->drawSES );
    this->drawSESParam << sespm;

    // ----- draw SAS param -----
    this->drawSAS = false;
    param::BoolParam *saspm = new param::BoolParam( this->drawSAS );
    this->drawSASParam << saspm;

    // fill amino acid color table
    this->FillAminoAcidColorTable();
    // fill rainbow color table
    this->MakeRainbowColorTable( 100);

    // set the FBOs and textures for post processing
    this->colorFBO = 0;
    this->blendFBO = 0;
    this->horizontalFilterFBO = 0;
    this->verticalFilterFBO = 0;
    this->texture0 = 0;
    this->depthTex0 = 0;
    this->hFilter = 0;
    this->vFilter = 0;
    // width and height of the screen
    this->width = 0;
    this->height = 0;
    
    // clear singularity texture
    this->singularityTexture.clear();
    // set singTexData to 0
    this->singTexData = 0;

    // clear cutting planes texture
    this->cutPlanesTexture.clear();
    // clear cutPlanesTexData 
    this->cutPlanesTexData.clear();
    
    this->preComputationDone = false;
    
    // export parameters
    this->MakeSlotAvailable( &this->rendermodeParam );
    this->MakeSlotAvailable( &this->postprocessingParam );
    this->MakeSlotAvailable( &this->coloringmodeParam );
    this->MakeSlotAvailable( &this->silhouettecolorParam );
    this->MakeSlotAvailable( &this->sigmaParam );
    this->MakeSlotAvailable( &this->lambdaParam );
    this->MakeSlotAvailable( &this->minvaluecolorParam );
    this->MakeSlotAvailable( &this->meanvaluecolorParam );
    this->MakeSlotAvailable( &this->maxvaluecolorParam );
    this->MakeSlotAvailable( &this->fogstartParam );
    this->MakeSlotAvailable( &this->debugParam );
    this->MakeSlotAvailable( &this->drawSESParam );
    this->MakeSlotAvailable( &this->drawSASParam );
}


/*
 * ProteinRendererBDP::~ProteinRendererBDP
 */
ProteinRendererBDP::~ProteinRendererBDP(void)
{
    if( colorFBO )
    {
        glDeleteFramebuffersEXT( 1, &colorFBO);
        glDeleteFramebuffersEXT( 1, &blendFBO);
        glDeleteFramebuffersEXT( 1, &horizontalFilterFBO);
        glDeleteFramebuffersEXT( 1, &verticalFilterFBO);
        glDeleteTextures( 1, &texture0);
        glDeleteTextures( 1, &depthTex0);
        glDeleteTextures( 1, &texture1);
        glDeleteTextures( 1, &depthTex1);
        glDeleteTextures( 1, &hFilter);
        glDeleteTextures( 1, &vFilter);
    }
    // delete singularity texture
    for( unsigned int i = 0; i < this->singularityTexture.size(); ++i )
        glDeleteTextures( 1, &this->singularityTexture[i]);
    // delete cutting planes textures and data
    for( unsigned int i = 0; i < this->cutPlanesTexture.size(); ++i )
        glDeleteTextures( 1, &this->cutPlanesTexture[i]);
    for( unsigned int i = 0; i < this->cutPlanesTexData.size(); ++i)
        this->cutPlanesTexData[i].Clear(true);
    // release
    this->cylinderShader.Release();
    this->sphereShader.Release();
    this->sphereClipInteriorShader.Release();
    this->sphericalTriangleShader.Release();
    this->torusShader.Release();
    this->lightShader.Release();
    this->hfilterShader.Release();
    this->vfilterShader.Release();
    this->silhouetteShader.Release();
    this->transparencyShader.Release();
    
    this->Release();
}


/*
 * protein::ProteinRendererBDP::release
 */
void ProteinRendererBDP::release( void )
{
    // intetionally empty
}


/*
 * ProteinRendererBDP::create
 */
bool ProteinRendererBDP::create( void )
{
    if( !glh_init_extensions( "GL_VERSION_2_0 GL_EXT_framebuffer_object GL_ARB_texture_float") )
        return false;
    
    if ( !vislib::graphics::gl::GLSLShader::InitialiseExtensions() )
        return false;
    
    //glEnable( GL_NORMALIZE);
    glEnable( GL_DEPTH_TEST);
    glDepthFunc( GL_LEQUAL);
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable( GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
    glEnable( GL_VERTEX_PROGRAM_TWO_SIDE);
    
    float spec[4] = { 1.0f, 1.0f, 1.0f, 1.0f};
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 50.0f);


    using namespace vislib::sys;
    using namespace vislib::graphics::gl;
    
    ShaderSource vertSrc;
    ShaderSource geomSrc;
    ShaderSource fragSrc;

    CoreInstance *ci = this->GetCoreInstance();
    if( !ci ) return false;
    
    ////////////////////////////////////////////////////
    // load the shader source for the sphere renderer //
    ////////////////////////////////////////////////////
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::bdp::sphereClipIntVertex", vertSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load vertex shader source for sphere shader", this->ClassName() );
        return false;
    }
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::bdp::sphereClipIntFragment", fragSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load fragment shader source for sphere shader", this->ClassName() );
        return false;
    }
    try
    {
        if( !this->sphereShader.Create( vertSrc.Code(), vertSrc.Count(), fragSrc.Code(), fragSrc.Count() ) )
        {
            throw vislib::Exception( "Generic creation failure", __FILE__, __LINE__ );
        }
    }
    catch( vislib::Exception e )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to create sphere shader: %s\n", this->ClassName(), e.GetMsgA() );
        return false;
    }

    ///////////////////////////////////////////////////
    // load the shader source for the torus renderer //
    ///////////////////////////////////////////////////
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::bdp::torusClipIntVertex", vertSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load vertex shader source for torus shader", this->ClassName() );
        return false;
    }
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::bdp::torusClipIntFragment", fragSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load fragment shader source for torus shader", this->ClassName() );
        return false;
    }
    try
    {
        if( !this->torusShader.Create( vertSrc.Code(), vertSrc.Count(), fragSrc.Code(), fragSrc.Count() ) )
        {
            throw vislib::Exception( "Generic creation failure", __FILE__, __LINE__ );
        }
    }
    catch( vislib::Exception e )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to create torus shader: %s\n", this->ClassName(), e.GetMsgA() );
        return false;
    }

    ////////////////////////////////////////////////////////////////
    // load the shader source for the spherical triangle renderer //
    ////////////////////////////////////////////////////////////////
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::bdp::sphericalTriaClipIntVertex", vertSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load vertex shader source for spherical triangle shader", this->ClassName() );
        return false;
    }
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::bdp::sphericalTriaClipIntFragment", fragSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load fragment shader source for spherical triangle shader", this->ClassName() );
        return false;
    }
    try
    {
        if( !this->sphericalTriangleShader.Create( vertSrc.Code(), vertSrc.Count(), fragSrc.Code(), fragSrc.Count() ) )
        {
            throw vislib::Exception( "Generic creation failure", __FILE__, __LINE__ );
        }
    }
    catch( vislib::Exception e )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to create spherical triangle shader: %s\n", this->ClassName(), e.GetMsgA() );
        return false;
    }
    
    //////////////////////////////////////////////////////
    // load the shader files for the per pixel lighting //
    //////////////////////////////////////////////////////
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::std::perpixellightVertex", vertSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load vertex shader source for per pixel lighting shader", this->ClassName() );
        return false;
    }
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::std::perpixellightFragment", fragSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load fragment shader source for per pixel lighting shader", this->ClassName() );
        return false;
    }
    try
    {
        if( !this->lightShader.Create( vertSrc.Code(), vertSrc.Count(), fragSrc.Code(), fragSrc.Count() ) )
        {
            throw vislib::Exception( "Generic creation failure", __FILE__, __LINE__ );
        }
    }
    catch( vislib::Exception e )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to create per pixel lighting shader: %s\n", this->ClassName(), e.GetMsgA() );
        return false;
    }

    /////////////////////////////////////////////////////////////////
    // load the shader files for horizontal 1D gaussian filtering  //
    /////////////////////////////////////////////////////////////////
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::std::hfilterVertex", vertSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load vertex shader source for horizontal 1D gaussian filter shader", this->ClassName() );
        return false;
    }
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::std::hfilterFragment", fragSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load fragment shader source for horizontal 1D gaussian filter shader", this->ClassName() );
        return false;
    }
    try
    {
        if( !this->hfilterShader.Create( vertSrc.Code(), vertSrc.Count(), fragSrc.Code(), fragSrc.Count() ) )
        {
            throw vislib::Exception( "Generic creation failure", __FILE__, __LINE__ );
        }
    }
    catch( vislib::Exception e )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to create horizontal 1D gaussian filter shader: %s\n", this->ClassName(), e.GetMsgA() );
        return false;
    }

    ///////////////////////////////////////////////////////////////
    // load the shader files for vertical 1D gaussian filtering  //
    ///////////////////////////////////////////////////////////////
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::std::vfilterVertex", vertSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load vertex shader source for vertical 1D gaussian filter shader", this->ClassName() );
        return false;
    }
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::std::vfilterFragment", fragSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load fragment shader source for vertical 1D gaussian filter shader", this->ClassName() );
        return false;
    }
    try
    {
        if( !this->vfilterShader.Create( vertSrc.Code(), vertSrc.Count(), fragSrc.Code(), fragSrc.Count() ) )
        {
            throw vislib::Exception( "Generic creation failure", __FILE__, __LINE__ );
        }
    }
    catch( vislib::Exception e )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to create vertical 1D gaussian filter shader: %s\n", this->ClassName(), e.GetMsgA() );
        return false;
    }

    //////////////////////////////////////////////////////
    // load the shader files for silhouette drawing     //
    //////////////////////////////////////////////////////
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::std::silhouetteVertex", vertSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load vertex shader source for silhouette drawing shader", this->ClassName() );
        return false;
    }
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::std::silhouetteFragment", fragSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load fragment shader source for silhouette drawing shader", this->ClassName() );
        return false;
    }
    try
    {
        if( !this->silhouetteShader.Create( vertSrc.Code(), vertSrc.Count(), fragSrc.Code(), fragSrc.Count() ) )
        {
            throw vislib::Exception( "Generic creation failure", __FILE__, __LINE__ );
        }
    }
    catch( vislib::Exception e )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to create vertical 1D gaussian filter shader: %s\n", this->ClassName(), e.GetMsgA() );
        return false;
    }
    
    //////////////////////////////////////////////////////
    // load the shader files for transparency           //
    //////////////////////////////////////////////////////
	/*
	// version 120
	fragSrc.Append(vertSrc.Append(new ShaderSource::VersionSnippet(120)));
	// vertex shader
	vertSrc.Append( this->shaderSnippetFactory().Create("transparency", 
            "protein", Utility::ShaderSnippetFactory::GLSL_VERTEX_SHADER));
	// fragment shader
	fragSrc.Append( this->shaderSnippetFactory().Create("transparency", 
            "protein", Utility::ShaderSnippetFactory::GLSL_FRAGMENT_SHADER));
	// create the shader
	this->transparencyShader.Create( vertSrc.Code(), vertSrc.Count(), fragSrc.Code(), fragSrc.Count());
	//clear fragment source
	fragSrc.Clear();
	//clear vertex source
	vertSrc.Clear();
	*/
    
    //////////////////////////////////////////////////////
    // load the shader source for the cylinder renderer //
    //////////////////////////////////////////////////////
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::std::cylinderVertex", vertSrc ) )
    {
        Log::DefaultLog.WriteMsg ( Log::LEVEL_ERROR, "%: Unable to load vertex shader source for cylinder shader", this->ClassName() );
        return false;
    }
    if( !ci->ShaderSourceFactory().MakeShaderSource( "protein::std::cylinderFragment", fragSrc ) )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to load vertex shader source for cylinder shader", this->ClassName() );
        return false;
    }
    try
    {
        if( !this->cylinderShader.Create( vertSrc.Code(), vertSrc.Count(), fragSrc.Code(), fragSrc.Count() ) )
        {
            throw vislib::Exception( "Generic creation failure", __FILE__, __LINE__ );
        }
    }
    catch( vislib::Exception e )
    {
        Log::DefaultLog.WriteMsg( Log::LEVEL_ERROR, "%s: Unable to create cylinder shader: %s\n", this->ClassName(), e.GetMsgA() );
        return false;
    }
    
    return true;
}


/*
 * ProteinRendererBDP::GetCapabilities
 */
bool ProteinRendererBDP::GetCapabilities(Call& call) {
    view::CallRender3D *cr3d = dynamic_cast<view::CallRender3D *>(&call);
    if (cr3d == NULL) return false;

    cr3d->SetCapabilities(view::CallRender3D::CAP_RENDER | view::CallRender3D::CAP_LIGHTING);

    return true;
}


/*
 * ProteinRendererBDP::GetExtents
 */
bool ProteinRendererBDP::GetExtents(Call& call) {
    view::CallRender3D *cr3d = dynamic_cast<view::CallRender3D *>(&call);
    if (cr3d == NULL) return false;

    protein::CallProteinData *protein = this->protDataCallerSlot.CallAs<protein::CallProteinData>();
    if (protein == NULL) return false;
    if (!(*protein)()) return false;

    float scale, xoff, yoff, zoff;
    vislib::math::Point<float, 3> bbc = protein->BoundingBox().CalcCenter();
    xoff = -bbc.X();
    yoff = -bbc.Y();
    zoff = -bbc.Z();
    scale = 2.0f / vislib::math::Max(vislib::math::Max(protein->BoundingBox().Width(),
        protein->BoundingBox().Height()), protein->BoundingBox().Depth());

    BoundingBoxes &bbox = cr3d->AccessBoundingBoxes();
    bbox.SetObjectSpaceBBox(protein->BoundingBox());
    bbox.SetWorldSpaceBBox(
        (protein->BoundingBox().Left() + xoff) * scale,
        (protein->BoundingBox().Bottom() + yoff) * scale,
        (protein->BoundingBox().Back() + zoff) * scale,
        (protein->BoundingBox().Right() + xoff) * scale,
        (protein->BoundingBox().Top() + yoff) * scale,
        (protein->BoundingBox().Front() + zoff) * scale);
    bbox.SetObjectSpaceClipBox(bbox.ObjectSpaceBBox());
    bbox.SetWorldSpaceClipBox(bbox.WorldSpaceBBox());

    return true;
}


/*
 * ProteinRendererBDP::Render
 */
bool ProteinRendererBDP::Render( Call& call )
{
    fpsCounter.FrameBegin();
    // temporary variables
    unsigned int cntRS = 0;
    bool recomputeColors = false;
    bool render_debug = false;
    
    // get pointer to CallProteinData
    protein::CallProteinData *protein = this->protDataCallerSlot.CallAs<protein::CallProteinData>();

    // if something went wrong --> return
    if( !protein) return false;

    // execute the call
    if ( ! ( *protein )() )
        return false;
    
    // get camera information
    this->cameraInfo = dynamic_cast<view::CallRender3D*>( &call )->GetCameraParameters();

    // ==================== check parameters ====================
    
    if ( this->postprocessingParam.IsDirty() )
    {
        this->postprocessing = static_cast<PostprocessingMode>(  this->postprocessingParam.Param<param::EnumParam>()->Value() );
        this->postprocessingParam.ResetDirty();
    }
    
    if( this->rendermodeParam.IsDirty() )
    {
        this->currentRendermode = static_cast<RenderMode>( this->rendermodeParam.Param<param::EnumParam>()->Value() );
        this->rendermodeParam.ResetDirty();
        this->preComputationDone = false;
    }
    
    if( this->coloringmodeParam.IsDirty() )
    {
        this->currentColoringMode = static_cast<ColoringMode>(  this->coloringmodeParam.Param<param::EnumParam>()->Value() );
        this->MakeColorTable( protein);
        this->preComputationDone = false;
        this->coloringmodeParam.ResetDirty();
    }
    
    if( this->silhouettecolorParam.IsDirty() )
    {
        this->SetSilhouetteColor( this->DecodeColor( this->silhouettecolorParam.Param<param::IntParam>()->Value() ) );
        this->silhouettecolorParam.ResetDirty();
    }
    
    if( this->sigmaParam.IsDirty() )
    {
        this->sigma = this->sigmaParam.Param<param::FloatParam>()->Value();
        this->sigmaParam.ResetDirty();
    }
    
    if( this->lambdaParam.IsDirty() )
    {
        this->lambda = this->lambdaParam.Param<param::FloatParam>()->Value();
        this->lambdaParam.ResetDirty();
    }
    
    if( this->minvaluecolorParam.IsDirty() )
    {
        this->SetMinValueColor( this->DecodeColor( this->minvaluecolorParam.Param<param::IntParam>()->Value() ) );
        this->minvaluecolorParam.ResetDirty();
        recomputeColors = true;
    }
    if( this->meanvaluecolorParam.IsDirty() )
    {
        this->SetMeanValueColor( this->DecodeColor( this->meanvaluecolorParam.Param<param::IntParam>()->Value() ) );
        recomputeColors = true;
    }
    if( this->maxvaluecolorParam.IsDirty() )
    {
        this->SetMaxValueColor( this->DecodeColor( this->maxvaluecolorParam.Param<param::IntParam>()->Value() ) );
        recomputeColors = true;
    }
    if( this->fogstartParam.IsDirty() )
    {
        this->fogStart = this->fogstartParam.Param<param::FloatParam>()->Value();
        this->fogstartParam.ResetDirty();
    }
    if( this->debugParam.IsDirty() )
    {
        this->drawRS = this->debugParam.Param<param::BoolParam>()->Value();
        this->debugParam.ResetDirty();
    }
    render_debug = this->drawRS;
    if( this->drawSESParam.IsDirty() )
    {
        this->drawSES = this->drawSESParam.Param<param::BoolParam>()->Value();
        this->drawSESParam.ResetDirty();
    }
    if( this->drawSASParam.IsDirty() )
    {
        this->drawSAS = this->drawSASParam.Param<param::BoolParam>()->Value();
        this->drawSASParam.ResetDirty();
        this->preComputationDone = false;
    }

    if( recomputeColors )
    {
        this->preComputationDone = false;
    }
    
    // ==================== Precomputations ====================
    
    if( this->currentRendermode == GPU_SIMPLIFIED )
    {
        //////////////////////////////////////////////////////////////////
        // Compute the simplified chains
        //////////////////////////////////////////////////////////////////
        for( cntRS = 0; cntRS < this->simpleRS.size(); ++cntRS )
        {
            delete this->simpleRS[cntRS];
        }
        this->simpleRS.clear();
        if( this->simpleRS.empty() )
        {
            this->simpleRS.push_back( new ReducedSurfaceSimplified( protein) );
        }
        // get minimum and maximum values for VALUE coloring mode
        this->minValue = protein->MinimumTemperatureFactor();
        this->maxValue = protein->MaximumTemperatureFactor();
        // compute the color table
        this->MakeColorTable( protein, true);
        // compute the arrays for ray casting
        this->ComputeRaycastingArraysSimple();
        
    }
    else
    {
        //////////////////////////////////////////////////////////////////////////////////////
        // TODO: handle both cases for reduced surface creation
        //////////////////////////////////////////////////////////////////////////////////////
        if( this->reducedSurface.empty() )
        {
            time_t t = clock();
            // create the reduced surface
            this->reducedSurface.push_back( new ReducedSurface( protein) );
            //unsigned int chainIds;
            //for( chainIds = 0; chainIds < protein->ProteinChainCount(); ++chainIds )
            //{
            //    this->reducedSurface.push_back( new ReducedSurface( chainIds, protein) );
            //}
            std::cout << "RS computed in: " << ( double( clock() - t) / double( CLOCKS_PER_SEC) ) << std::endl;
        }
        // update the data / the RS
        for( cntRS = 0; cntRS < this->reducedSurface.size(); ++cntRS )
        {
            if( this->reducedSurface[cntRS]->UpdateData( 1.0f, 5.0f) )
            {
                this->ComputeRaycastingArrays( cntRS);
            }
        }
        
    }
    
    if( !this->preComputationDone )
    {
        // get minimum and maximum values for VALUE coloring mode
        this->minValue = protein->MinimumTemperatureFactor();
        this->maxValue = protein->MaximumTemperatureFactor();
        // compute the color table
        this->MakeColorTable( protein, recomputeColors);
        // compute the data needed for the current render mode
        if( this->currentRendermode == GPU_RAYCASTING )
            this->ComputeRaycastingArrays();
        // set the precomputation of the data as done
        this->preComputationDone = true;
    }
    
    if( this->postprocessing != NONE && (
        static_cast<unsigned int>(cameraInfo->VirtualViewSize().GetWidth()) != this->width ||
        static_cast<unsigned int>(cameraInfo->VirtualViewSize().GetHeight()) != this->height ) )
    {
        this->width = static_cast<unsigned int>(cameraInfo->VirtualViewSize().GetWidth());
        this->height = static_cast<unsigned int>(cameraInfo->VirtualViewSize().GetHeight());
        this->CreateFBO();
    }
    
    // ==================== Scale & Translate ====================
    
    glPushMatrix();

    float scale, xoff, yoff, zoff;
    vislib::math::Point<float, 3> bbc = protein->BoundingBox().CalcCenter();

    xoff = -bbc.X();
    yoff = -bbc.Y();
    zoff = -bbc.Z();

    scale = 2.0f / vislib::math::Max ( vislib::math::Max ( protein->BoundingBox().Width(),
                                       protein->BoundingBox().Height() ), protein->BoundingBox().Depth() );

    glScalef ( scale, scale, scale );
    glTranslatef ( xoff, yoff, zoff );
    
    // ==================== Start actual rendering ====================
    
    glDisable( GL_BLEND);
    //glEnable( GL_NORMALIZE);
    glEnable( GL_DEPTH_TEST);
    glEnable( GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
    glEnable( GL_VERTEX_PROGRAM_TWO_SIDE);
    
    if( this->postprocessing == TRANSPARENCY )
    {
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, this->blendFBO);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if( render_debug )
            this->RenderDebugStuff( protein);
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0);
    }
    else
    {
        if( render_debug ) {
            this->RenderDebugStuff( protein);
            // DEMO
            glPopMatrix();
            fpsCounter.FrameEnd();
            std::cout << "average fps: " << fpsCounter.FPS() << std::endl;
            return true;
        }
    }
    
    // start rendering to frame buffer object
    if( this->postprocessing != NONE )
    {
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, this->colorFBO);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    // render the SES
    if( this->currentRendermode == GPU_RAYCASTING )
    {
        this->RenderSESGpuRaycasting( protein);
    }
    else if( this->currentRendermode == GPU_SIMPLIFIED )
    {
        // render the simplified SES via GPU ray casting
        this->RenderSESGpuRaycastingSimple( protein);
    }

    //////////////////////////////////
    // apply postprocessing effects //
    //////////////////////////////////
    if( this->postprocessing != NONE )
    {
        // stop rendering to frame buffer object
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0);

        if( this->postprocessing == AMBIENT_OCCLUSION )
            this->PostprocessingSSAO();
        else if( this->postprocessing == SILHOUETTE )
            this->PostprocessingSilhouette();
        else if( this->postprocessing == TRANSPARENCY )
            this->PostprocessingTransparency( 0.5f);
    }
    
    glPopMatrix();
    
    fpsCounter.FrameEnd();
    std::cout << "average fps: " << fpsCounter.FPS() << std::endl;

    return true;
}


/*
 * postprocessing: use screen space ambient occlusion
 */
void ProteinRendererBDP::PostprocessingSSAO()
{
    // START draw overlay
    glBindTexture( GL_TEXTURE_2D, this->depthTex0);
    // --> this seems to be unnecessary since no mipmap but the original resolution is used
    //glGenerateMipmapEXT( GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glOrtho( 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);

    glPushAttrib( GL_LIGHTING_BIT);
    glDisable( GL_LIGHTING);

    // ----- START gaussian filtering + SSAO -----
    // apply horizontal filter
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, this->horizontalFilterFBO);
    
    this->hfilterShader.Enable();

    glUniform1fARB( this->hfilterShader.ParameterLocation( "sigma"), this->sigma);
    glColor4f( 1.0f,  1.0f,  1.0f,  1.0f);
    glBegin(GL_QUADS);
    glVertex2f( 0.0f, 0.0f);
    glVertex2f( 1.0f, 0.0f);
    glVertex2f( 1.0f, 1.0f);
    glVertex2f( 0.0f, 1.0f);
    glEnd();

    this->hfilterShader.Disable();

    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0);

    // apply vertical filter to horizontally filtered image and compute colors
    glBindTexture( GL_TEXTURE_2D, this->hFilter);
    glActiveTexture( GL_TEXTURE1);
    glBindTexture( GL_TEXTURE_2D, this->texture0);

    this->vfilterShader.Enable();

    glUniform1iARB( this->vfilterShader.ParameterLocation( "tex"), 0);
    glUniform1iARB( this->vfilterShader.ParameterLocation( "colorTex"), 1);
    glUniform1fARB( this->vfilterShader.ParameterLocation( "sigma"), this->sigma);
    glUniform1fARB( this->vfilterShader.ParameterLocation( "lambda"), this->lambda);
    glColor4f( 1.0f,  1.0f,  1.0f,  1.0f);
    glBegin(GL_QUADS);
    glVertex2f( 0.0f, 0.0f);
    glVertex2f( 1.0f, 0.0f);
    glVertex2f( 1.0f, 1.0f);
    glVertex2f( 0.0f, 1.0f);
    glEnd();

    this->vfilterShader.Disable();
    // ----- END gaussian filtering + SSAO -----

    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glBindTexture( GL_TEXTURE_2D, 0);
    glActiveTexture( GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, 0);
    // END draw overlay
}


/*
 * postprocessing: use silhouette shader
 */
void ProteinRendererBDP::PostprocessingSilhouette()
{
    // START draw overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glOrtho( 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);

    glPushAttrib( GL_LIGHTING_BIT);
    glDisable( GL_LIGHTING);

    // ----- START -----
    glBindTexture( GL_TEXTURE_2D, this->depthTex0);
    glActiveTexture( GL_TEXTURE1);
    glBindTexture( GL_TEXTURE_2D, this->texture0);

    this->silhouetteShader.Enable();

    glUniform1iARB( this->silhouetteShader.ParameterLocation( "tex"), 0);
    glUniform1iARB( this->silhouetteShader.ParameterLocation( "colorTex"), 1);
    glUniform1fARB( this->silhouetteShader.ParameterLocation( "difference"), 0.025f);
    glColor4f( this->silhouetteColor.GetX(), this->silhouetteColor.GetY(), 
        this->silhouetteColor.GetZ(),  1.0f);
    glBegin(GL_QUADS);
    glVertex2f( 0.0f, 0.0f);
    glVertex2f( 1.0f, 0.0f);
    glVertex2f( 1.0f, 1.0f);
    glVertex2f( 0.0f, 1.0f);
    glEnd();

    this->silhouetteShader.Disable();
    // ----- END -----

    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glBindTexture( GL_TEXTURE_2D, 0);
    glActiveTexture( GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, 0);
    // END draw overlay
}


/*
 * postprocessing: transparency (blend two images)
 */
void ProteinRendererBDP::PostprocessingTransparency( float transparency)
{
    // START draw overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glOrtho( 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);

    glPushAttrib( GL_LIGHTING_BIT);
    glDisable( GL_LIGHTING);

    // ----- START -----
    glBindTexture( GL_TEXTURE_2D, this->depthTex0);
    glActiveTexture( GL_TEXTURE1);
    glBindTexture( GL_TEXTURE_2D, this->texture0);
    glActiveTexture( GL_TEXTURE2);
    glBindTexture( GL_TEXTURE_2D, this->depthTex1);
    glActiveTexture( GL_TEXTURE3);
    glBindTexture( GL_TEXTURE_2D, this->texture1);

    this->transparencyShader.Enable();

    glUniform1iARB( this->transparencyShader.ParameterLocation( "depthTex0"), 0);
    glUniform1iARB( this->transparencyShader.ParameterLocation( "colorTex0"), 1);
    glUniform1iARB( this->transparencyShader.ParameterLocation( "depthTex1"), 2);
    glUniform1iARB( this->transparencyShader.ParameterLocation( "colorTex1"), 3);
    glUniform1fARB( this->transparencyShader.ParameterLocation( "transparency"), transparency);
    glBegin(GL_QUADS);
    glVertex2f( 0.0f, 0.0f);
    glVertex2f( 1.0f, 0.0f);
    glVertex2f( 1.0f, 1.0f);
    glVertex2f( 0.0f, 1.0f);
    glEnd();

    this->transparencyShader.Disable();
    // ----- END -----

    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glBindTexture( GL_TEXTURE_2D, 0);
    glActiveTexture( GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, 0);
    // END draw overlay
}


/*
 * Create the fbo and texture needed for offscreen rendering
 */
void ProteinRendererBDP::CreateFBO()
{
    if( colorFBO )
    {
        glDeleteFramebuffersEXT( 1, &colorFBO);
        glDeleteFramebuffersEXT( 1, &blendFBO);
        glDeleteFramebuffersEXT( 1, &horizontalFilterFBO);
        glDeleteFramebuffersEXT( 1, &verticalFilterFBO);
        glDeleteTextures( 1, &texture0);
        glDeleteTextures( 1, &depthTex0);
        glDeleteTextures( 1, &texture1);
        glDeleteTextures( 1, &depthTex1);
        glDeleteTextures( 1, &hFilter);
        glDeleteTextures( 1, &vFilter);
    }
    glGenFramebuffersEXT( 1, &colorFBO);
    glGenFramebuffersEXT( 1, &blendFBO);
    glGenFramebuffersEXT( 1, &horizontalFilterFBO);
    glGenFramebuffersEXT( 1, &verticalFilterFBO);
    glGenTextures( 1, &texture0);
    glGenTextures( 1, &depthTex0);
    glGenTextures( 1, &texture1);
    glGenTextures( 1, &depthTex1);
    glGenTextures( 1, &hFilter);
    glGenTextures( 1, &vFilter);
    
    // color and depth FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, this->colorFBO);
    // init texture0 (color)
    glBindTexture( GL_TEXTURE_2D, texture0);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16_EXT, this->width, this->height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture( GL_TEXTURE_2D, 0);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture0, 0);
    // init depth texture
    glBindTexture( GL_TEXTURE_2D, depthTex0);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, this->width, this->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, this->depthTex0, 0);
    glBindTexture( GL_TEXTURE_2D, 0);
    
    // color and depth FBO for blending (transparency)
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, this->blendFBO);
    // init texture1 (color)
    glBindTexture( GL_TEXTURE_2D, texture1);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16_EXT, this->width, this->height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture( GL_TEXTURE_2D, 0);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture1, 0);
    // init depth texture
    glBindTexture( GL_TEXTURE_2D, depthTex1);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, this->width, this->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, this->depthTex1, 0);
    glBindTexture( GL_TEXTURE_2D, 0);

    //horizontal filter FBO
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, this->horizontalFilterFBO);
    glBindTexture( GL_TEXTURE_2D, this->hFilter);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16_EXT, this->width, this->height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, hFilter, 0);
    glBindTexture( GL_TEXTURE_2D, 0);
    
    //vertical filter FBO
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, this->verticalFilterFBO);
    glBindTexture( GL_TEXTURE_2D, this->vFilter);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16_EXT, this->width, this->height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, vFilter, 0);
    glBindTexture( GL_TEXTURE_2D, 0);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}


/*
 * Render the molecular surface using GPU raycasting
 */
void ProteinRendererBDP::RenderSESGpuRaycasting(
    const CallProteinData *protein)
{
    // TODO: attribute locations nicht jedes mal neu abfragen!

    // set viewport
    float viewportStuff[4] =
    {
        this->cameraInfo->TileRect().Left(),
        this->cameraInfo->TileRect().Bottom(),
        this->cameraInfo->TileRect().Width(),
        this->cameraInfo->TileRect().Height()
    };
    if (viewportStuff[2] < 1.0f) viewportStuff[2] = 1.0f;
    if (viewportStuff[3] < 1.0f) viewportStuff[3] = 1.0f;
    viewportStuff[2] = 2.0f / viewportStuff[2];
    viewportStuff[3] = 2.0f / viewportStuff[3];

    // get clear color (i.e. background color) for fogging
    float *clearColor = new float[4];
    glGetFloatv( GL_COLOR_CLEAR_VALUE, clearColor);
    vislib::math::Vector<float, 3> fogCol( clearColor[0], clearColor[1], clearColor[2]);
    
    unsigned int cntRS;
    
    for( cntRS = 0; cntRS < this->reducedSurface.size(); ++cntRS )
    {
        
        if( this->drawSES ) {
            //////////////////////////////////
            // ray cast the tori on the GPU //
            //////////////////////////////////
            /*
            // enable torus shader
            this->torusShader.Enable();
            // set shader variables
            glUniform4fvARB( this->torusShader.ParameterLocation( "viewAttr"), 1, viewportStuff);
            glUniform3fvARB( this->torusShader.ParameterLocation( "camIn"), 1, cameraInfo->Front().PeekComponents());
            glUniform3fvARB( this->torusShader.ParameterLocation( "camRight"), 1, cameraInfo->Right().PeekComponents());
            glUniform3fvARB( this->torusShader.ParameterLocation( "camUp"), 1, cameraInfo->Up().PeekComponents());
            glUniform3fARB( this->torusShader.ParameterLocation( "zValues"), fogStart, cameraInfo->NearClip(), cameraInfo->FarClip());
            glUniform3fARB( this->torusShader.ParameterLocation( "fogCol"), fogCol.GetX(), fogCol.GetY(), fogCol.GetZ() );
            glUniform1fARB( this->torusShader.ParameterLocation( "alpha"), this->transparency);
            // get attribute locations
            GLuint attribInParams = glGetAttribLocationARB( this->torusShader, "inParams");
            GLuint attribQuatC = glGetAttribLocationARB( this->torusShader, "quatC");
            GLuint attribInSphere = glGetAttribLocationARB( this->torusShader, "inSphere");
            GLuint attribInColors = glGetAttribLocationARB( this->torusShader, "inColors");
            GLuint attribInCuttingPlane = glGetAttribLocationARB( this->torusShader, "inCuttingPlane");
            // set color to orange
            glColor3f( 1.0f, 0.75f, 0.0f);
            glEnableClientState( GL_VERTEX_ARRAY);
            // enable vertex attribute arrays for the attribute locations
            glEnableVertexAttribArrayARB( attribInParams);
            glEnableVertexAttribArrayARB( attribQuatC);
            glEnableVertexAttribArrayARB( attribInSphere);
            glEnableVertexAttribArrayARB( attribInColors);
            glEnableVertexAttribArrayARB( attribInCuttingPlane);
            // set vertex and attribute pointers and draw them
            glVertexAttribPointerARB( attribInParams, 3, GL_FLOAT, 0, 0, this->torusInParamArray[cntRS].PeekElements());
            glVertexAttribPointerARB( attribQuatC, 4, GL_FLOAT, 0, 0, this->torusQuatCArray[cntRS].PeekElements());
            glVertexAttribPointerARB( attribInSphere, 4, GL_FLOAT, 0, 0, this->torusInSphereArray[cntRS].PeekElements());
            glVertexAttribPointerARB( attribInColors, 4, GL_FLOAT, 0, 0, this->torusColors[cntRS].PeekElements());
            glVertexAttribPointerARB( attribInCuttingPlane, 3, GL_FLOAT, 0, 0, this->torusInCuttingPlaneArray[cntRS].PeekElements());
            glVertexPointer( 3, GL_FLOAT, 0, this->torusVertexArray[cntRS].PeekElements());
            glDrawArrays( GL_POINTS, 0, ((unsigned int)this->torusVertexArray[cntRS].Count())/3);

            // disable vertex attribute arrays for the attribute locations
            glDisableVertexAttribArrayARB( attribInParams);
            glDisableVertexAttribArrayARB( attribQuatC);
            glDisableVertexAttribArrayARB( attribInSphere);
            glDisableVertexAttribArrayARB( attribInColors);
            glDisableVertexAttribArrayARB( attribInCuttingPlane);
            glDisableClientState(GL_VERTEX_ARRAY);
            // enable torus shader
            this->torusShader.Disable();
            */
            
            /////////////////////////////////////////////////
            // ray cast the spherical triangles on the GPU //
            /////////////////////////////////////////////////
            
            // bind texture
            glBindTexture( GL_TEXTURE_2D, this->singularityTexture[cntRS]);
            // enable spherical triangle shader
            this->sphericalTriangleShader.Enable();
            // set shader variables
            glUniform4fvARB( this->sphericalTriangleShader.ParameterLocation("viewAttr"), 1, viewportStuff);
            glUniform3fvARB( this->sphericalTriangleShader.ParameterLocation("camIn"), 1, cameraInfo->Front().PeekComponents());
            glUniform3fvARB( this->sphericalTriangleShader.ParameterLocation("camRight"), 1, cameraInfo->Right().PeekComponents());
            glUniform3fvARB( this->sphericalTriangleShader.ParameterLocation("camUp"), 1, cameraInfo->Up().PeekComponents());
            glUniform3fARB( this->sphericalTriangleShader.ParameterLocation( "zValues"), fogStart, cameraInfo->NearClip(), cameraInfo->FarClip());
            glUniform3fARB( this->sphericalTriangleShader.ParameterLocation( "fogCol"), fogCol.GetX(), fogCol.GetY(), fogCol.GetZ() );
            glUniform2fARB( this->sphericalTriangleShader.ParameterLocation( "texOffset"),
                                 1.0f/(float)this->singTexWidth[cntRS], 1.0f/(float)this->singTexHeight[cntRS] );
            glUniform1fARB( this->sphericalTriangleShader.ParameterLocation( "alpha"), this->transparency);
            // get attribute locations
            GLuint attribVec1 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribVec1");
            GLuint attribVec2 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribVec2");
            GLuint attribVec3 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribVec3");
            GLuint attribTexCoord1 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribTexCoord1");
            GLuint attribTexCoord2 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribTexCoord2");
            GLuint attribTexCoord3 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribTexCoord3");
            GLuint attribColors = glGetAttribLocationARB( this->sphericalTriangleShader, "attribColors");
            GLuint attribFrontBack = glGetAttribLocationARB( this->sphericalTriangleShader, "attribFrontBack");
            // set color to turquoise (= Tuerkis)
            glColor3f( 0.0f, 0.75f, 1.0f);
            glEnableClientState( GL_VERTEX_ARRAY);
            // enable vertex attribute arrays for the attribute locations
            glEnableVertexAttribArrayARB( attribVec1);
            glEnableVertexAttribArrayARB( attribVec2);
            glEnableVertexAttribArrayARB( attribVec3);
            glEnableVertexAttribArrayARB( attribTexCoord1);
            glEnableVertexAttribArrayARB( attribTexCoord2);
            glEnableVertexAttribArrayARB( attribTexCoord3);
            glEnableVertexAttribArrayARB( attribColors);
            glEnableVertexAttribArrayARB( attribFrontBack);
            // set vertex and attribute pointers and draw them
            glVertexAttribPointerARB( attribVec1, 4, GL_FLOAT, 0, 0, this->sphericTriaVec1[cntRS].PeekElements());
            glVertexAttribPointerARB( attribVec2, 4, GL_FLOAT, 0, 0, this->sphericTriaVec2[cntRS].PeekElements());
            glVertexAttribPointerARB( attribVec3, 4, GL_FLOAT, 0, 0, this->sphericTriaVec3[cntRS].PeekElements());
            glVertexAttribPointerARB( attribTexCoord1, 3, GL_FLOAT, 0, 0, this->sphericTriaTexCoord1[cntRS].PeekElements());
            glVertexAttribPointerARB( attribTexCoord2, 3, GL_FLOAT, 0, 0, this->sphericTriaTexCoord2[cntRS].PeekElements());
            glVertexAttribPointerARB( attribTexCoord3, 3, GL_FLOAT, 0, 0, this->sphericTriaTexCoord3[cntRS].PeekElements());
            glVertexAttribPointerARB( attribColors, 3, GL_FLOAT, 0, 0, this->sphericTriaColors[cntRS].PeekElements());
            glVertexPointer( 4, GL_FLOAT, 0, this->sphericTriaVertexArray[cntRS].PeekElements());
            glDrawArrays( GL_POINTS, 0, ((unsigned int)this->sphericTriaVertexArray[cntRS].Count())/4);

            // disable vertex attribute arrays for the attribute locations
            glDisableVertexAttribArrayARB( attribVec1);
            glDisableVertexAttribArrayARB( attribVec2);
            glDisableVertexAttribArrayARB( attribVec3);
            glDisableVertexAttribArrayARB( attribTexCoord1);
            glDisableVertexAttribArrayARB( attribTexCoord2);
            glDisableVertexAttribArrayARB( attribTexCoord3);
            glDisableVertexAttribArrayARB( attribColors);
            glDisableClientState(GL_VERTEX_ARRAY);
            // disable spherical triangle shader
            this->sphericalTriangleShader.Disable();
            // unbind texture
            glBindTexture( GL_TEXTURE_2D, 0);
            
        }
        

        /////////////////////////////////////
        // ray cast the spheres on the GPU //
        /////////////////////////////////////
        
        // bind texture
        glBindTexture( GL_TEXTURE_2D, this->cutPlanesTexture[cntRS]);
        // enable sphere shader
        this->sphereShader.Enable();
        // set shader variables
        glUniform4fvARB( this->sphereShader.ParameterLocation("viewAttr"), 1, viewportStuff);
        glUniform3fvARB( this->sphereShader.ParameterLocation("camIn"), 1, cameraInfo->Front().PeekComponents());
        glUniform3fvARB( this->sphereShader.ParameterLocation("camRight"), 1, cameraInfo->Right().PeekComponents());
        glUniform3fvARB( this->sphereShader.ParameterLocation("camUp"), 1, cameraInfo->Up().PeekComponents());
        glUniform3fARB( this->sphereShader.ParameterLocation( "zValues"), fogStart, cameraInfo->NearClip(), cameraInfo->FarClip());
        glUniform3fARB( this->sphereShader.ParameterLocation( "fogCol"), fogCol.GetX(), fogCol.GetY(), fogCol.GetZ() );
        glUniform2fARB( this->sphereShader.ParameterLocation( "texOffset"),
                             1.0f/(float)this->cutPlanesTexWidth[cntRS], 1.0f/(float)this->cutPlanesTexHeight[cntRS] );
        glUniform1fARB( this->sphereShader.ParameterLocation( "alpha"), this->transparency);
        // get attribute locations
        GLuint attribTexCoord = glGetAttribLocationARB( this->sphereShader, "attribTexCoord");
        GLuint attribSurfVector = glGetAttribLocationARB( this->sphereShader,  "attribSurfVector");
        // enable vertex attribute arrays for the attribute locations
        glEnableClientState( GL_VERTEX_ARRAY);
        glEnableClientState( GL_COLOR_ARRAY);
        glEnableVertexAttribArrayARB( attribTexCoord);
        glEnableVertexAttribArrayARB( attribSurfVector);
        // set vertex, color and attribute pointers and draw them
        glVertexAttribPointerARB( attribTexCoord, 3, GL_FLOAT, 0, 0, this->sphereTexCoord[cntRS].PeekElements());
        glVertexAttribPointerARB( attribSurfVector, 4, GL_FLOAT, 0, 0, this->sphereSurfVector[cntRS].PeekElements());
        glColorPointer( 3, GL_FLOAT, 0, this->sphereColors[cntRS].PeekElements());
        glVertexPointer( 4, GL_FLOAT, 0, this->sphereVertexArray[cntRS].PeekElements());
        glDrawArrays( GL_POINTS, 0, ((unsigned int)this->sphereVertexArray[cntRS].Count())/4);

        // disable vertex attribute arrays for the attribute locations
        glDisableVertexAttribArrayARB( attribTexCoord);
        glDisableVertexAttribArrayARB( attribSurfVector);
        glDisableClientState( GL_COLOR_ARRAY);
        glDisableClientState( GL_VERTEX_ARRAY);
        // disable sphere shader
        this->sphereShader.Disable();
        // unbind texture
        glBindTexture( GL_TEXTURE_2D, 0);

        // DEBUG
        // drawing suurfVectors as yellow lines
        glDisable( GL_LIGHTING);
        glLineWidth( 1.0f);
        glEnable( GL_LINE_WIDTH);
        glPushAttrib( GL_POLYGON_BIT);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
        unsigned int i;
        for( i = 0; i < ((unsigned int)this->sphereVertexArray[cntRS].Count())/4; i++ )
        {
            glColor3f( 1.0f, 1.0f, 0.0f);
            glBegin( GL_LINES );
                glVertex3f( this->sphereVertexArray[cntRS][4*i], this->sphereVertexArray[cntRS][4*i+1], this->sphereVertexArray[cntRS][4*i+2]);
                glVertex3f( this->sphereVertexArray[cntRS][4*i] + this->sphereSurfVector[cntRS][4*i]*1.5f, 
                            this->sphereVertexArray[cntRS][4*i+1] + this->sphereSurfVector[cntRS][4*i+1]*1.5f, 
                            this->sphereVertexArray[cntRS][4*i+2] + this->sphereSurfVector[cntRS][4*i+2]*1.5f);
            glEnd(); //GL_LINES
        }
        glPopAttrib();
        glDisable( GL_LINE_WIDTH);
        glEnable( GL_LIGHTING);
        // end DEBUG
        
    }
            
    // delete pointers
    delete[] clearColor;
}


/*
 * Render the molecular surface using GPU raycasting
 */
void ProteinRendererBDP::RenderSESGpuRaycastingSimple(
    const CallProteinData *protein)
{
    // TODO: attribute locations nicht jedes mal neu abfragen!

    // set viewport
    float viewportStuff[4] =
    {
        cameraInfo->TileRect().Left(),
        cameraInfo->TileRect().Bottom(),
        cameraInfo->TileRect().Width(),
        cameraInfo->TileRect().Height()
    };
    if (viewportStuff[2] < 1.0f) viewportStuff[2] = 1.0f;
    if (viewportStuff[3] < 1.0f) viewportStuff[3] = 1.0f;
    viewportStuff[2] = 2.0f / viewportStuff[2];
    viewportStuff[3] = 2.0f / viewportStuff[3];

    // get clear color (i.e. background color) for fogging
    float *clearColor = new float[4];
    glGetFloatv( GL_COLOR_CLEAR_VALUE, clearColor);
    vislib::math::Vector<float, 3> fogCol( clearColor[0], clearColor[1], clearColor[2]);
    
    unsigned int cntRS;
    
    for( cntRS = 0; cntRS < this->simpleRS.size(); ++cntRS )
    {
        //////////////////////////////////
        // ray cast the tori on the GPU //
        //////////////////////////////////
        // enable torus shader
        this->torusShader.Enable();
        // set shader variables
        glUniform4fvARB( this->torusShader.ParameterLocation( "viewAttr"), 1, viewportStuff);
        glUniform3fvARB( this->torusShader.ParameterLocation( "camIn"), 1, cameraInfo->Front().PeekComponents());
        glUniform3fvARB( this->torusShader.ParameterLocation( "camRight"), 1, cameraInfo->Right().PeekComponents());
        glUniform3fvARB( this->torusShader.ParameterLocation( "camUp"), 1, cameraInfo->Up().PeekComponents());
        glUniform3fARB( this->torusShader.ParameterLocation( "zValues"), fogStart, cameraInfo->NearClip(), cameraInfo->FarClip());
        glUniform3fARB( this->torusShader.ParameterLocation( "fogCol"), fogCol.GetX(), fogCol.GetY(), fogCol.GetZ() );
        glUniform1fARB( this->torusShader.ParameterLocation( "alpha"), this->transparency);
        // get attribute locations
        GLuint attribInParams = glGetAttribLocationARB( this->torusShader, "inParams");
        GLuint attribQuatC = glGetAttribLocationARB( this->torusShader, "quatC");
        GLuint attribInSphere = glGetAttribLocationARB( this->torusShader, "inSphere");
        GLuint attribInColors = glGetAttribLocationARB( this->torusShader, "inColors");
        GLuint attribInCuttingPlane = glGetAttribLocationARB( this->torusShader, "inCuttingPlane");
        // set color to orange
        glColor3f( 1.0f, 0.75f, 0.0f);
        glEnableClientState( GL_VERTEX_ARRAY);
        // enable vertex attribute arrays for the attribute locations
        glEnableVertexAttribArrayARB( attribInParams);
        glEnableVertexAttribArrayARB( attribQuatC);
        glEnableVertexAttribArrayARB( attribInSphere);
        glEnableVertexAttribArrayARB( attribInColors);
        glEnableVertexAttribArrayARB( attribInCuttingPlane);
        // set vertex and attribute pointers and draw them
        glVertexAttribPointerARB( attribInParams, 3, GL_FLOAT, 0, 0, this->torusInParamArray[cntRS].PeekElements());
        glVertexAttribPointerARB( attribQuatC, 4, GL_FLOAT, 0, 0, this->torusQuatCArray[cntRS].PeekElements());
        glVertexAttribPointerARB( attribInSphere, 4, GL_FLOAT, 0, 0, this->torusInSphereArray[cntRS].PeekElements());
        glVertexAttribPointerARB( attribInColors, 4, GL_FLOAT, 0, 0, this->torusColors[cntRS].PeekElements());
        glVertexAttribPointerARB( attribInCuttingPlane, 3, GL_FLOAT, 0, 0, this->torusInCuttingPlaneArray[cntRS].PeekElements());
        glVertexPointer( 3, GL_FLOAT, 0, this->torusVertexArray[cntRS].PeekElements());
        glDrawArrays( GL_POINTS, 0, ((unsigned int)this->torusVertexArray[cntRS].Count())/3);
        // disable vertex attribute arrays for the attribute locations
        glDisableVertexAttribArrayARB( attribInParams);
        glDisableVertexAttribArrayARB( attribQuatC);
        glDisableVertexAttribArrayARB( attribInSphere);
        glDisableVertexAttribArrayARB( attribInColors);
        glDisableVertexAttribArrayARB( attribInCuttingPlane);
        glDisableClientState(GL_VERTEX_ARRAY);
        // enable torus shader
        this->torusShader.Disable();

        /////////////////////////////////////////////////
        // ray cast the spherical triangles on the GPU //
        /////////////////////////////////////////////////
        // bind texture
        glBindTexture( GL_TEXTURE_2D, singularityTexture[cntRS]);
        // enable spherical triangle shader
        this->sphericalTriangleShader.Enable();
        // set shader variables
        glUniform4fvARB( this->sphericalTriangleShader.ParameterLocation("viewAttr"), 1, viewportStuff);
        glUniform3fvARB( this->sphericalTriangleShader.ParameterLocation("camIn"), 1, cameraInfo->Front().PeekComponents());
        glUniform3fvARB( this->sphericalTriangleShader.ParameterLocation("camRight"), 1, cameraInfo->Right().PeekComponents());
        glUniform3fvARB( this->sphericalTriangleShader.ParameterLocation("camUp"), 1, cameraInfo->Up().PeekComponents());
        glUniform3fARB( this->sphericalTriangleShader.ParameterLocation( "zValues"), fogStart, cameraInfo->NearClip(), cameraInfo->FarClip());
        glUniform3fARB( this->sphericalTriangleShader.ParameterLocation( "fogCol"), fogCol.GetX(), fogCol.GetY(), fogCol.GetZ() );
        glUniform2fARB( this->sphericalTriangleShader.ParameterLocation( "texOffset"),
                             1.0f/(float)this->singTexWidth[cntRS], 1.0f/(float)this->singTexHeight[cntRS] );
        glUniform1fARB( this->sphericalTriangleShader.ParameterLocation( "alpha"), this->transparency);
        // get attribute locations
        GLuint attribVec1 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribVec1");
        GLuint attribVec2 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribVec2");
        GLuint attribVec3 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribVec3");
        GLuint attribTexCoord1 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribTexCoord1");
        GLuint attribTexCoord2 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribTexCoord2");
        GLuint attribTexCoord3 = glGetAttribLocationARB( this->sphericalTriangleShader, "attribTexCoord3");
        GLuint attribColors = glGetAttribLocationARB( this->sphericalTriangleShader, "attribColors");
        // set color to turquoise
        glColor3f( 0.0f, 0.75f, 1.0f);
        glEnableClientState( GL_VERTEX_ARRAY);
        // enable vertex attribute arrays for the attribute locations
        glEnableVertexAttribArrayARB( attribVec1);
        glEnableVertexAttribArrayARB( attribVec2);
        glEnableVertexAttribArrayARB( attribVec3);
        glEnableVertexAttribArrayARB( attribTexCoord1);
        glEnableVertexAttribArrayARB( attribTexCoord2);
        glEnableVertexAttribArrayARB( attribTexCoord3);
        glEnableVertexAttribArrayARB( attribColors);
        // set vertex and attribute pointers and draw them
        glVertexAttribPointerARB( attribVec1, 4, GL_FLOAT, 0, 0, this->sphericTriaVec1[cntRS].PeekElements());
        glVertexAttribPointerARB( attribVec2, 4, GL_FLOAT, 0, 0, this->sphericTriaVec2[cntRS].PeekElements());
        glVertexAttribPointerARB( attribVec3, 4, GL_FLOAT, 0, 0, this->sphericTriaVec3[cntRS].PeekElements());
        glVertexAttribPointerARB( attribTexCoord1, 3, GL_FLOAT, 0, 0, this->sphericTriaTexCoord1[cntRS].PeekElements());
        glVertexAttribPointerARB( attribTexCoord2, 3, GL_FLOAT, 0, 0, this->sphericTriaTexCoord2[cntRS].PeekElements());
        glVertexAttribPointerARB( attribTexCoord3, 3, GL_FLOAT, 0, 0, this->sphericTriaTexCoord3[cntRS].PeekElements());
        glVertexAttribPointerARB( attribColors, 3, GL_FLOAT, 0, 0, this->sphericTriaColors[cntRS].PeekElements());
        glVertexPointer( 4, GL_FLOAT, 0, this->sphericTriaVertexArray[cntRS].PeekElements());
        glDrawArrays( GL_POINTS, 0, ((unsigned int)this->sphericTriaVertexArray[cntRS].Count())/4);
        // disable vertex attribute arrays for the attribute locations
        glDisableVertexAttribArrayARB( attribVec1);
        glDisableVertexAttribArrayARB( attribVec2);
        glDisableVertexAttribArrayARB( attribVec3);
        glDisableVertexAttribArrayARB( attribTexCoord1);
        glDisableVertexAttribArrayARB( attribTexCoord2);
        glDisableVertexAttribArrayARB( attribTexCoord3);
        glDisableVertexAttribArrayARB( attribColors);
        glDisableClientState(GL_VERTEX_ARRAY);
        // disable spherical triangle shader
        this->sphericalTriangleShader.Disable();
        // unbind texture
        glBindTexture( GL_TEXTURE_2D, 0);
    
        /////////////////////////////////////
        // ray cast the spheres on the GPU //
        /////////////////////////////////////
        // enable sphere shader
        this->sphereShader.Enable();
        glEnableClientState( GL_VERTEX_ARRAY);
        glEnableClientState( GL_COLOR_ARRAY);
        // set shader variables
        glUniform4fvARB( this->sphereShader.ParameterLocation("viewAttr"), 1, viewportStuff);
        glUniform3fvARB( this->sphereShader.ParameterLocation("camIn"), 1, cameraInfo->Front().PeekComponents());
        glUniform3fvARB( this->sphereShader.ParameterLocation("camRight"), 1, cameraInfo->Right().PeekComponents());
        glUniform3fvARB( this->sphereShader.ParameterLocation("camUp"), 1, cameraInfo->Up().PeekComponents());
        glUniform3fARB( this->sphereShader.ParameterLocation( "zValues"), fogStart, cameraInfo->NearClip(), cameraInfo->FarClip());
        glUniform3fARB( this->sphereShader.ParameterLocation( "fogCol"), fogCol.GetX(), fogCol.GetY(), fogCol.GetZ() );
        glUniform1fARB( this->sphereShader.ParameterLocation( "alpha"), this->transparency);
        // set vertex and color pointers and draw them
        glColorPointer( 3, GL_FLOAT, 0, this->sphereColors[cntRS].PeekElements());
        glVertexPointer( 4, GL_FLOAT, 0, this->sphereVertexArray[cntRS].PeekElements());
        glDrawArrays( GL_POINTS, 0, ((unsigned int)this->sphereVertexArray[cntRS].Count())/4);
        // disable sphere shader
        glDisableClientState( GL_COLOR_ARRAY);
        glDisableClientState( GL_VERTEX_ARRAY);
        // disable sphere shader
        this->sphereShader.Disable();
    }
            
    // delete pointers
    delete[] clearColor;
}


/*
 * Render debug stuff
 */
void ProteinRendererBDP::RenderDebugStuff(
    const CallProteinData *protein)
{
    // --> USAGE: UNCOMMENT THE NEEDED PARTS
    
    // temporary variables
    unsigned int max1, max2;
    max1 = max2 = 0;
    vislib::math::Vector<float, 3> v1, v2, v3, n1;
    v1.Set( 0, 0, 0);
    v2 = v3 = n1 = v1;

    //////////////////////////////////////////////////////////////////////////
    // Render the atom positions as GL_POINTS
    //////////////////////////////////////////////////////////////////////////
    /*
    glDisable( GL_LIGHTING);
    glEnable( GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_POINT_SIZE);
    glEnable( GL_POINT_SMOOTH);
    glPointSize( 5.0f);
    glBegin( GL_POINTS);
    glColor3f( 1.0f, 0.0f, 0.0f);
    if( this->currentRendermode == GPU_SIMPLIFIED )
        max1 = this->simpleRS.size();
    else
        max1 = this->reducedSurface.size();
    for( unsigned int cntRS = 0; cntRS < max1; ++cntRS)
    {
        if( this->currentRendermode == GPU_SIMPLIFIED )
            max2 = this->simpleRS[cntRS]->GetRSVertexCount();
        else
            max2 = this->reducedSurface[cntRS]->GetRSVertexCount();
        for( unsigned int i = 0; i < max2; ++i )
        {
            if( this->currentRendermode == GPU_SIMPLIFIED )
                v1 = this->simpleRS[cntRS]->GetRSVertex( i)->GetPosition();
            else
                v1 = this->reducedSurface[cntRS]->GetRSVertex( i)->GetPosition();
            glVertex3fv( v1.PeekComponents());
        }
    }
    glEnd(); // GL_POINTS
    glDisable( GL_POINT_SMOOTH);
    glDisable( GL_POINT_SIZE);
    glDisable( GL_BLEND);
    glEnable( GL_LIGHTING);
    */
    
    //////////////////////////////////////////////////////////////////////////
    // Render the probe positions
    //////////////////////////////////////////////////////////////////////////
    /*
    glDisable( GL_LIGHTING);
    glEnable( GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_POINT_SIZE);
    glEnable( GL_POINT_SMOOTH);
    glPointSize( 5.0f);
    for( unsigned int i = 0; i < this->rsFace.size(); ++i )
    {
        glColor3f( 1.0f, 0.0f, 0.0f);
        glBegin( GL_POINTS);
            glVertex3fv( this->rsFace[i]->GetProbeCenter().PeekComponents());
        glEnd(); // GL_POINTS            
    }
    glDisable( GL_POINT_SMOOTH);
    glDisable( GL_POINT_SIZE);
    glDisable( GL_BLEND);
    glEnable( GL_LIGHTING);
    */

    /*
    //////////////////////////////////////////////////////////////////////////
    // Render the probes that cut edges
    //////////////////////////////////////////////////////////////////////////
    // set viewport
    float viewportStuff[4] =
    {
        cameraInfo->TileRect().Left(),
        cameraInfo->TileRect().Bottom(),
        cameraInfo->TileRect().Width(),
        cameraInfo->TileRect().Height()
    };
    if (viewportStuff[2] < 1.0f) viewportStuff[2] = 1.0f;
    if (viewportStuff[3] < 1.0f) viewportStuff[3] = 1.0f;
    viewportStuff[2] = 2.0f / viewportStuff[2];
    viewportStuff[3] = 2.0f / viewportStuff[3];
    // enable sphere shader
    this->sphereShader.Enable();
    // set shader variables
    glUniform4fvARB(this->sphereShader.ParameterLocation("viewAttr"), 1, viewportStuff);
    glUniform3fvARB(this->sphereShader.ParameterLocation("camIn"), 1, cameraInfo->Front().PeekComponents());
    glUniform3fvARB(this->sphereShader.ParameterLocation("camRight"), 1, cameraInfo->Right().PeekComponents());
    glUniform3fvARB(this->sphereShader.ParameterLocation("camUp"), 1, cameraInfo->Up().PeekComponents());
    glColor3f( 0.8f, 0.8f, 0.8f);
    unsigned int i, j;
    glBegin( GL_POINTS);
    for( i = 0; i < this->rsEdge.size(); ++i )
    {
        for( j = 0; j < this->rsEdge[i]->cuttingProbes.size(); ++j )
        {
            glVertex4f( this->rsEdge[i]->cuttingProbes[j]->GetProbeCenter().GetX(),
                this->rsEdge[i]->cuttingProbes[j]->GetProbeCenter().GetY(),
                this->rsEdge[i]->cuttingProbes[j]->GetProbeCenter().GetZ(),
                this->probeRadius);
        }
    }
    glEnd(); // GL_POINTS
    sphereShader.Disable();
    */

    //////////////////////////////////////////////////////////////////////////
    // Draw reduced surface
    //////////////////////////////////////////////////////////////////////////
    this->RenderAtomsGPU( protein, 0.2f);
    vislib::math::Quaternion<float> quatC;
    quatC.Set( 0, 0, 0, 1);
    vislib::math::Vector<float, 3> firstAtomPos, secondAtomPos;
    vislib::math::Vector<float,3> tmpVec, ortho, dir, position;
    float angle;
    // set viewport
    float viewportStuff[4] =
    {
        cameraInfo->TileRect().Left(),
        cameraInfo->TileRect().Bottom(),
        cameraInfo->TileRect().Width(),
        cameraInfo->TileRect().Height()
    };
    if (viewportStuff[2] < 1.0f) viewportStuff[2] = 1.0f;
    if (viewportStuff[3] < 1.0f) viewportStuff[3] = 1.0f;
    viewportStuff[2] = 2.0f / viewportStuff[2];
    viewportStuff[3] = 2.0f / viewportStuff[3];
    // enable cylinder shader
    this->cylinderShader.Enable();
    // set shader variables
    glUniform4fvARB(this->cylinderShader.ParameterLocation("viewAttr"), 1, viewportStuff);
    glUniform3fvARB(this->cylinderShader.ParameterLocation("camIn"), 1, cameraInfo->Front().PeekComponents());
    glUniform3fvARB(this->cylinderShader.ParameterLocation("camRight"), 1, cameraInfo->Right().PeekComponents());
    glUniform3fvARB(this->cylinderShader.ParameterLocation("camUp"), 1, cameraInfo->Up().PeekComponents());
    // get the attribute locations
    GLint attribLocInParams = glGetAttribLocation( this->cylinderShader, "inParams");
    GLint attribLocQuatC = glGetAttribLocation( this->cylinderShader, "quatC");
    GLint attribLocColor1 = glGetAttribLocation( this->cylinderShader, "color1");
    GLint attribLocColor2 = glGetAttribLocation( this->cylinderShader, "color2");
    glBegin( GL_POINTS);
    if( this->currentRendermode == GPU_SIMPLIFIED )
        max1 = (unsigned int)this->simpleRS.size();
    else
        max1 = (unsigned int)this->reducedSurface.size();
    for( unsigned int cntRS = 0; cntRS < max1; ++cntRS)
    {
        if( this->currentRendermode == GPU_SIMPLIFIED )
            max2 = this->simpleRS[cntRS]->GetRSEdgeCount();
        else
            max2 = this->reducedSurface[cntRS]->GetRSEdgeCount();
        for( unsigned int j = 0; j < max2; ++j )
        {
            if( this->currentRendermode == GPU_SIMPLIFIED )
            {
                firstAtomPos = this->simpleRS[cntRS]->GetRSEdge( j)->GetVertex1()->GetPosition();
                secondAtomPos = this->simpleRS[cntRS]->GetRSEdge( j)->GetVertex2()->GetPosition();
            }
            else
            {
                firstAtomPos = this->reducedSurface[cntRS]->GetRSEdge( j)->GetVertex1()->GetPosition();
                secondAtomPos = this->reducedSurface[cntRS]->GetRSEdge( j)->GetVertex2()->GetPosition();
            }
    
            // compute the quaternion for the rotation of the cylinder
            dir = secondAtomPos - firstAtomPos;
            tmpVec.Set( 1.0f, 0.0f, 0.0f);
            angle = - tmpVec.Angle( dir);
            ortho = tmpVec.Cross( dir);
            ortho.Normalise();
            quatC.Set( angle, ortho);
            // compute the absolute position 'position' of the cylinder (center point)
            position = firstAtomPos + ( dir/2.0f);
                    
            // draw vertex and attributes
            glVertexAttrib2f( attribLocInParams, 0.12f, (firstAtomPos-secondAtomPos).Length() );
            glVertexAttrib4fv( attribLocQuatC, quatC.PeekComponents());
            glVertexAttrib3f( attribLocColor1, 1.0f, 0.5f, 0.0f);
            glVertexAttrib3f( attribLocColor2, 1.0f, 0.5f, 0.0f);
            glVertex4f( position.GetX(), position.GetY(), position.GetZ(), 1.0f);
        }
    }
    glEnd(); // GL_POINTS
    // disable cylinder shader
    this->cylinderShader.Disable();

    glEnable( GL_COLOR_MATERIAL);
    this->lightShader.Enable();
    unsigned int i;
    for( unsigned int cntRS = 0; cntRS < max1; ++cntRS)
    {
        if( this->currentRendermode == GPU_SIMPLIFIED )
            max2 = this->simpleRS[cntRS]->GetRSFaceCount();
        else
            max2 = this->reducedSurface[cntRS]->GetRSFaceCount();
        for( i = 0; i < max2; ++i )
        {
            if( this->currentRendermode == GPU_SIMPLIFIED )
            {
                n1 = this->simpleRS[cntRS]->GetRSFace( i)->GetFaceNormal();
                v1 = this->simpleRS[cntRS]->GetRSFace( i)->GetVertex1()->GetPosition();
                v2 = this->simpleRS[cntRS]->GetRSFace( i)->GetVertex2()->GetPosition();
                v3 = this->simpleRS[cntRS]->GetRSFace( i)->GetVertex3()->GetPosition();
            }
            else
            {
                n1 = this->reducedSurface[cntRS]->GetRSFace( i)->GetFaceNormal();
                v1 = this->reducedSurface[cntRS]->GetRSFace( i)->GetVertex1()->GetPosition();
                v2 = this->reducedSurface[cntRS]->GetRSFace( i)->GetVertex2()->GetPosition();
                v3 = this->reducedSurface[cntRS]->GetRSFace( i)->GetVertex3()->GetPosition();
            }
                
            glBegin( GL_TRIANGLES );
                glNormal3fv( n1.PeekComponents());
                glColor3f( 1.0f, 0.8f, 0.0f);
                glVertex3fv( v1.PeekComponents());
                //glColor3f( 0.0f, 0.7f, 0.7f);
                glVertex3fv( v2.PeekComponents());
                //glColor3f( 0.7f, 0.0f, 0.7f);
                glVertex3fv( v3.PeekComponents());
            glEnd(); //GL_TRIANGLES
        }
    }
    this->lightShader.Disable();
    glDisable( GL_COLOR_MATERIAL);
        
    /*
    //////////////////////////////////////////////////////////////////////////
    // Draw double edges as thick lines
    //////////////////////////////////////////////////////////////////////////
    glDisable( GL_LIGHTING);
    glLineWidth( 5.0f);
    glEnable( GL_LINE_WIDTH);

    glPushAttrib( GL_POLYGON_BIT);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
    unsigned int i;
    for( i = 0; i < this->doubleEdgeList.size(); i=i+2 )
    {
        glColor3f( 1.0f, 1.0f, 0.0f);
        glBegin( GL_LINES );
            glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetVertex1()->GetIndex()].PeekComponents());
            glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetVertex2()->GetIndex()].PeekComponents());
        glEnd(); //GL_LINES
        glBegin( GL_TRIANGLES);
        // draw the coincident face
        if( this->doubleEdgeList[i]->GetFace1() == this->doubleEdgeList[i+1]->GetFace1() ||
            this->doubleEdgeList[i]->GetFace1() == this->doubleEdgeList[i+1]->GetFace2() )
        {
            glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace1()->GetVertex1()->GetIndex()].PeekComponents() );
            glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace1()->GetVertex2()->GetIndex()].PeekComponents() );
            glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace1()->GetVertex3()->GetIndex()].PeekComponents() );
            //draw the inner and outer faces
            if( fabs( this->doubleEdgeList[i]->GetRotationAngle() ) < fabs( this->doubleEdgeList[i+1]->GetRotationAngle() ) )
            {
                glColor3f( 0.0f, 1.0f, 0.0f);
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace2()->GetVertex1()->GetIndex()].PeekComponents() );
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace2()->GetVertex2()->GetIndex()].PeekComponents() );
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace2()->GetVertex3()->GetIndex()].PeekComponents() );
                glColor3f( 1.0f, 0.0f, 0.0f);
                if( this->doubleEdgeList[i]->GetFace1() == this->doubleEdgeList[i+1]->GetFace1() )
                {
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex1()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex2()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex3()->GetIndex()].PeekComponents() );
                }
                else
                {
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex1()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex2()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex3()->GetIndex()].PeekComponents() );
                }
            }
            else
            {
                glColor3f( 1.0f, 0.0f, 0.0f);
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace2()->GetVertex1()->GetIndex()].PeekComponents() );
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace2()->GetVertex2()->GetIndex()].PeekComponents() );
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace2()->GetVertex3()->GetIndex()].PeekComponents() );
                glColor3f( 0.0f, 1.0f, 0.0f);
                if( this->doubleEdgeList[i]->GetFace1() == this->doubleEdgeList[i+1]->GetFace1() )
                {
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex1()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex2()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex3()->GetIndex()].PeekComponents() );
                }
                else
                {
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex1()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex2()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex3()->GetIndex()].PeekComponents() );
                }
            }
        }
        else
        {
            glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace2()->GetVertex1()->GetIndex()].PeekComponents() );
            glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace2()->GetVertex2()->GetIndex()].PeekComponents() );
            glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace2()->GetVertex3()->GetIndex()].PeekComponents() );
            //draw the inner and outer faces
            if( fabs( this->doubleEdgeList[i]->GetRotationAngle() ) < fabs( this->doubleEdgeList[i+1]->GetRotationAngle() ) )
            {
                glColor3f( 0.0f, 1.0f, 0.0f);
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace1()->GetVertex1()->GetIndex()].PeekComponents() );
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace1()->GetVertex2()->GetIndex()].PeekComponents() );
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace1()->GetVertex3()->GetIndex()].PeekComponents() );
                glColor3f( 1.0f, 0.0f, 0.0f);
                if( this->doubleEdgeList[i]->GetFace2() == this->doubleEdgeList[i+1]->GetFace1() )
                {
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex1()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex2()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex3()->GetIndex()].PeekComponents() );
                }
                else
                {
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex1()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex2()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex3()->GetIndex()].PeekComponents() );
                }
            }
            else
            {
                glColor3f( 1.0f, 0.0f, 0.0f);
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace1()->GetVertex1()->GetIndex()].PeekComponents() );
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace1()->GetVertex2()->GetIndex()].PeekComponents() );
                glVertex3fv( this->atomPos[this->doubleEdgeList[i]->GetFace1()->GetVertex3()->GetIndex()].PeekComponents() );
                glColor3f( 0.0f, 1.0f, 0.0f);
                if( this->doubleEdgeList[i]->GetFace2() == this->doubleEdgeList[i+1]->GetFace1() )
                {
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex1()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex2()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace2()->GetVertex3()->GetIndex()].PeekComponents() );
                }
                else
                {
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex1()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex2()->GetIndex()].PeekComponents() );
                    glVertex3fv( this->atomPos[this->doubleEdgeList[i+1]->GetFace1()->GetVertex3()->GetIndex()].PeekComponents() );
                }
            }
        }
        glEnd(); // GL_TRIANGLES
    }
    glPopAttrib();

    glDisable( GL_LINE_WIDTH);
    glEnable( GL_LIGHTING);
    */

    /*
    //////////////////////////////////////////////////////////////////////////
    // Draw tetrahedra defining the convace spherical triangles 
    //////////////////////////////////////////////////////////////////////////
    glDisable( GL_LIGHTING);
    glLineWidth( 3.0f);
    glEnable( GL_LINE_WIDTH);
    for( i = 0; i < this->rsFace.size(); ++i )
    {
        glColor3f( 1.0f, 1.0f, 0.0f);
        glBegin( GL_LINES );
            glVertex3fv( this->atomPos[this->rsFace[i]->GetVertex1()->GetIndex()].PeekComponents());
            glVertex3fv( this->rsFace[i]->GetProbeCenter().PeekComponents());
            glVertex3fv( this->atomPos[this->rsFace[i]->GetVertex2()->GetIndex()].PeekComponents());
            glVertex3fv( this->rsFace[i]->GetProbeCenter().PeekComponents());
            glVertex3fv( this->atomPos[this->rsFace[i]->GetVertex3()->GetIndex()].PeekComponents());
            glVertex3fv( this->rsFace[i]->GetProbeCenter().PeekComponents());
        glEnd(); //GL_LINES
    }
    glDisable( GL_LINE_WIDTH);
    glEnable( GL_LIGHTING);
    */

    /*
    for( i = 0; i < this->rsFace.size(); ++i )
    {
        this->RenderProbe( this->rsFace[i]->GetProbeCenter(), info);
    }
    */

    /*
    //////////////////////////////////////////////////////////////////////////
    // Draw edges 
    //////////////////////////////////////////////////////////////////////////
    glPushAttrib( GL_LINE_BIT);
    glLineWidth( 3.0f);
    glEnable( GL_LINE_WIDTH);
    glDisable( GL_LIGHTING);
    glBegin( GL_LINES);
    for( i = 0; i < this->rsEdge.size(); ++i )
    {
        if( this->rsEdge[i]->GetFace2() < 0 || 
            ( *this->rsEdge[i]->GetFace1() == *this->rsEdge[i]->GetFace2() ) )
        {
            glColor3f( 1.0f, 0.0f, 1.0f);
            //std::cout << "Edge: " << i << " (" << 
            //    this->rsEdge[i]->GetFace1() << ")--(" << this->rsEdge[i]->GetFace2() << ") " <<
            //    this->rsEdge[i].GetRotationAngle() << std::endl;
        }
        else
        {
            glColor3f( 0.0f, 1.0f, 0.0f);
        }
        glVertex3fv( this->atomPos[this->rsEdge[i]->GetVertex1()->GetIndex()].PeekComponents());
        glVertex3fv( this->atomPos[this->rsEdge[i]->GetVertex2()->GetIndex()].PeekComponents());
    }
    glEnd(); // GL_LINES
    glEnable( GL_LIGHTING);
    glDisable( GL_LINE_WIDTH);
    glPopAttrib();
    */

    /*
    //////////////////////////////////////////////////////////////////////////
    // Draw lines between probes
    //////////////////////////////////////////////////////////////////////////
    glPushAttrib( GL_LINE_BIT);
    glEnable( GL_LINE_WIDTH);
    glLineWidth( 5.0f);
    glDisable( GL_LIGHTING);
    glBegin( GL_LINES);
    for( i = 0; i < this->rsEdge.size(); ++i )
    {
        glColor3f( 1.0f, 1.0f, 0.0f);
        glVertex3fv( this->rsFace[this->rsEdge[i]->GetFace1()]->GetProbeCenter().PeekComponents());
        glVertex3fv( this->rsFace[this->rsEdge[i]->GetFace2()]->GetProbeCenter().PeekComponents());
    }
    glEnd(); // GL_LINES
    glEnable( GL_LIGHTING);
    glDisable( GL_LINE_WIDTH);
    glPopAttrib();
    */
}


/*
 * Compute the vertex and attribute arrays for the raycasting shaders
 * (spheres, spherical triangles & tori)
 */
void ProteinRendererBDP::ComputeRaycastingArrays()
{    
    //time_t t = clock();

    unsigned int cntRS;
    unsigned int i;
    
    // resize lists of vertex, attribute and color arrays
    this->sphericTriaVertexArray.resize( this->reducedSurface.size());
    this->sphericTriaVec1.resize( this->reducedSurface.size());
    this->sphericTriaVec2.resize( this->reducedSurface.size());
    this->sphericTriaVec3.resize( this->reducedSurface.size());
    this->sphericTriaTexCoord1.resize( this->reducedSurface.size());
    this->sphericTriaTexCoord2.resize( this->reducedSurface.size());
    this->sphericTriaTexCoord3.resize( this->reducedSurface.size());
    this->sphericTriaColors.resize( this->reducedSurface.size());
    this->torusVertexArray.resize( this->reducedSurface.size());
    this->torusInParamArray.resize( this->reducedSurface.size());
    this->torusQuatCArray.resize( this->reducedSurface.size());
    this->torusInSphereArray.resize( this->reducedSurface.size());
    this->torusColors.resize( this->reducedSurface.size());
    this->torusInCuttingPlaneArray.resize( this->reducedSurface.size());
    this->sphereVertexArray.resize( this->reducedSurface.size());
    this->sphereColors.resize( this->reducedSurface.size());
    this->sphereTexCoord.resize( this->reducedSurface.size());
    this->sphereSurfVector.resize( this->reducedSurface.size());
    
    // compute singulatity textures
    this->CreateSingularityTextures();
    
    // compute cutting planes textures (needs outNormals!)
    this->CreateCutPlanesTextures();

    for( cntRS = 0; cntRS < this->reducedSurface.size(); ++cntRS )
    {
        ///////////////////////////////////////////////////////////////////////
        // compute arrays for ray casting the spherical triangles on the GPU //
        ///////////////////////////////////////////////////////////////////////
        vislib::math::Vector<float, 3> tmpVec;
        vislib::math::Vector<float, 3> tmpDualProbe( 1.0f, 1.0f, 1.0f);
        float dualProbeRad = 0.0f;
        
        this->sphericTriaVertexArray[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSFaceCount() * 4);
        this->sphericTriaVec1[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSFaceCount() * 4);
        this->sphericTriaVec2[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSFaceCount() * 4);
        this->sphericTriaVec3[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSFaceCount() * 4);
        this->sphericTriaTexCoord1[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSFaceCount() * 3);
        this->sphericTriaTexCoord2[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSFaceCount() * 3);
        this->sphericTriaTexCoord3[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSFaceCount() * 3);
        this->sphericTriaColors[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSFaceCount() * 3);
    
        // loop over all RS-faces
        for( i = 0; i < this->reducedSurface[cntRS]->GetRSFaceCount(); ++i )
        {
            // if the face has a dual face --> store the probe of this face
            if( this->reducedSurface[cntRS]->GetRSFace( i)->GetDualFace() != NULL )
            {
                tmpDualProbe = this->reducedSurface[cntRS]->GetRSFace( i)->GetDualFace()->GetProbeCenter();
                dualProbeRad = this->probeRadius;
            }
            // first RS-vertex
            tmpVec = this->reducedSurface[cntRS]->GetRSFace( i)->GetVertex1()->GetPosition() - this->reducedSurface[cntRS]->GetRSFace( i)->GetProbeCenter();
            this->sphericTriaVec1[cntRS][i*4+0] = tmpVec.GetX();
            this->sphericTriaVec1[cntRS][i*4+1] = tmpVec.GetY();
            this->sphericTriaVec1[cntRS][i*4+2] = tmpVec.GetZ();
            this->sphericTriaVec1[cntRS][i*4+3] = 1.0f;
            // second RS-vertex
            tmpVec = this->reducedSurface[cntRS]->GetRSFace( i)->GetVertex2()->GetPosition() - this->reducedSurface[cntRS]->GetRSFace( i)->GetProbeCenter();
            this->sphericTriaVec2[cntRS][i*4+0] = tmpVec.GetX();
            this->sphericTriaVec2[cntRS][i*4+1] = tmpVec.GetY();
            this->sphericTriaVec2[cntRS][i*4+2] = tmpVec.GetZ();
            this->sphericTriaVec2[cntRS][i*4+3] = 1.0f;
            // third RS-vertex
            tmpVec = this->reducedSurface[cntRS]->GetRSFace( i)->GetVertex3()->GetPosition() - this->reducedSurface[cntRS]->GetRSFace( i)->GetProbeCenter();
            this->sphericTriaVec3[cntRS][i*4+0] = tmpVec.GetX();
            this->sphericTriaVec3[cntRS][i*4+1] = tmpVec.GetY();
            this->sphericTriaVec3[cntRS][i*4+2] = tmpVec.GetZ();
            this->sphericTriaVec3[cntRS][i*4+3] = dualProbeRad*dualProbeRad;
            // store number of cutting probes and texture coordinates for each edge
            this->sphericTriaTexCoord1[cntRS][i*3+0] = (float)this->reducedSurface[cntRS]->GetRSFace( i)->GetEdge1()->cuttingProbes.size();
            this->sphericTriaTexCoord1[cntRS][i*3+1] = (float)this->reducedSurface[cntRS]->GetRSFace( i)->GetEdge1()->GetTexCoordX();
            this->sphericTriaTexCoord1[cntRS][i*3+2] = (float)this->reducedSurface[cntRS]->GetRSFace( i)->GetEdge1()->GetTexCoordY();
            this->sphericTriaTexCoord2[cntRS][i*3+0] = (float)this->reducedSurface[cntRS]->GetRSFace( i)->GetEdge2()->cuttingProbes.size();
            this->sphericTriaTexCoord2[cntRS][i*3+1] = (float)this->reducedSurface[cntRS]->GetRSFace( i)->GetEdge2()->GetTexCoordX();
            this->sphericTriaTexCoord2[cntRS][i*3+2] = (float)this->reducedSurface[cntRS]->GetRSFace( i)->GetEdge2()->GetTexCoordY();
            this->sphericTriaTexCoord3[cntRS][i*3+0] = (float)this->reducedSurface[cntRS]->GetRSFace( i)->GetEdge3()->cuttingProbes.size();
            this->sphericTriaTexCoord3[cntRS][i*3+1] = (float)this->reducedSurface[cntRS]->GetRSFace( i)->GetEdge3()->GetTexCoordX();
            this->sphericTriaTexCoord3[cntRS][i*3+2] = (float)this->reducedSurface[cntRS]->GetRSFace( i)->GetEdge3()->GetTexCoordY();
            // colors
            this->sphericTriaColors[cntRS][i*3+0] = CodeColor( this->atomColor[this->reducedSurface[cntRS]->GetRSFace( i)->GetVertex1()->GetIndex()]);
            this->sphericTriaColors[cntRS][i*3+1] = CodeColor( this->atomColor[this->reducedSurface[cntRS]->GetRSFace( i)->GetVertex2()->GetIndex()]);
            this->sphericTriaColors[cntRS][i*3+2] = CodeColor( this->atomColor[this->reducedSurface[cntRS]->GetRSFace( i)->GetVertex3()->GetIndex()]);
            // sphere center
            this->sphericTriaVertexArray[cntRS][i*4+0] = this->reducedSurface[cntRS]->GetRSFace( i)->GetProbeCenter().GetX();
            this->sphericTriaVertexArray[cntRS][i*4+1] = this->reducedSurface[cntRS]->GetRSFace( i)->GetProbeCenter().GetY();
            this->sphericTriaVertexArray[cntRS][i*4+2] = this->reducedSurface[cntRS]->GetRSFace( i)->GetProbeCenter().GetZ();
            this->sphericTriaVertexArray[cntRS][i*4+3] = this->GetProbeRadius();
        }
    
        ////////////////////////////////////////////////////////
        // compute arrays for ray casting the tori on the GPU //
        ////////////////////////////////////////////////////////
        vislib::math::Quaternion<float> quatC;
        vislib::math::Vector<float, 3> zAxis, torusAxis, rotAxis, P, X1, X2, C, planeNormal;
        zAxis.Set( 0.0f, 0.0f, 1.0f);
        float distance, d;
        vislib::math::Vector<float, 3> tmpDir1, tmpDir2, tmpDir3, cutPlaneNorm;
    
        this->torusVertexArray[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSEdgeCount() * 3);
        this->torusInParamArray[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSEdgeCount() * 3);
        this->torusQuatCArray[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSEdgeCount() * 4);
        this->torusInSphereArray[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSEdgeCount() * 4);
        this->torusColors[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSEdgeCount() * 4);
        this->torusInCuttingPlaneArray[cntRS].SetCount( this->reducedSurface[cntRS]->GetRSEdgeCount() * 3);
    
        // loop over all RS-edges
        for( i = 0; i < this->reducedSurface[cntRS]->GetRSEdgeCount(); ++i )
        {
            // get the rotation axis of the torus
            torusAxis = this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusCenter();
            torusAxis.Normalise();
            // get the axis for rotating the torus rotations axis on the z-axis
            rotAxis = torusAxis.Cross( zAxis);
            rotAxis.Normalise();
            // compute quaternion
            quatC.Set( torusAxis.Angle( zAxis), rotAxis);
            // compute the tangential point X2 of the spheres
            P = this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusCenter() + rotAxis *
                this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusRadius();
    
            X1 = P - this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition();
            X1.Normalise();
            X1 *= this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex1()->GetRadius();
            X2 = P - this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition();
            X2.Normalise();
            X2 *= this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex2()->GetRadius();
            d = ( X1 + this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusCenter()).Dot( torusAxis);
    
            C = this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition() -
                    this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition();
            C = ( ( P - this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition()).Length() /
                    ( ( P - this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition()).Length() + 
                    ( P - this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition()).Length())) * C;
            distance = ( X2 - C).Length();
            C = ( C + this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition()) -
                    this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusCenter();
            
            // compute normal of the cutting plane
            tmpDir1 = this->reducedSurface[cntRS]->GetRSEdge( i)->GetFace1()->GetProbeCenter();
            tmpDir2 = this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition() - tmpDir1;
            tmpDir2.Normalise();
            tmpDir2 *= this->probeRadius;
            tmpDir2 = tmpDir2 + tmpDir1 - this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusCenter();
            tmpDir3 = this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - tmpDir1;
            tmpDir3.Normalise();
            tmpDir3 *= this->probeRadius;
            tmpDir3 = tmpDir3 + tmpDir1 - this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusCenter();
            // tmpDir2 and tmpDir3 now store the position of the intersection points for face 1
            cutPlaneNorm = tmpDir2 - tmpDir3;
            // cutPlaneNorm now stores the vector between the two intersection points for face 1
            tmpDir1 = this->reducedSurface[cntRS]->GetRSEdge( i)->GetFace2()->GetProbeCenter();
            tmpDir2 = this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - tmpDir1;
            tmpDir2.Normalise();
            tmpDir2 *= this->probeRadius;
            tmpDir2 = tmpDir2 + tmpDir1 - this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusCenter();
            // tmpDir2 now stores the position of the intersection point 1 for face 2
            tmpDir2 = tmpDir2 - tmpDir3;
            // tmpDir2 and tmpDir3 now span the plane containing the four intersection points
            cutPlaneNorm = cutPlaneNorm.Cross( tmpDir2);
            cutPlaneNorm = torusAxis.Cross( cutPlaneNorm);
            cutPlaneNorm.Normalise();
            
            // attributes
            this->torusInParamArray[cntRS][i*3+0] = this->probeRadius;
            this->torusInParamArray[cntRS][i*3+1] = this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusRadius();
            this->torusInParamArray[cntRS][i*3+2] = this->reducedSurface[cntRS]->GetRSEdge( i)->GetRotationAngle();
            this->torusQuatCArray[cntRS][i*4+0] = quatC.GetX();
            this->torusQuatCArray[cntRS][i*4+1] = quatC.GetY();
            this->torusQuatCArray[cntRS][i*4+2] = quatC.GetZ();
            this->torusQuatCArray[cntRS][i*4+3] = quatC.GetW();
            this->torusInSphereArray[cntRS][i*4+0] = C.GetX();
            this->torusInSphereArray[cntRS][i*4+1] = C.GetY();
            this->torusInSphereArray[cntRS][i*4+2] = C.GetZ();
            this->torusInSphereArray[cntRS][i*4+3] = distance;
            // colors
            this->torusColors[cntRS][i*4+0] = CodeColor( this->atomColor[this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex1()->GetIndex()]);
            this->torusColors[cntRS][i*4+1] = CodeColor( this->atomColor[this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex2()->GetIndex()]);
            this->torusColors[cntRS][i*4+2] = d;
            //this->torusColors[cntRS][i*4+3] = ( X2 - X1).Length();
            this->torusColors[cntRS][i*4+3] = ( X2 + this->reducedSurface[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition() - this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusCenter()).Dot( torusAxis) - d;
            // cutting plane
            this->torusInCuttingPlaneArray[cntRS][i*3+0] = cutPlaneNorm.GetX();
            this->torusInCuttingPlaneArray[cntRS][i*3+1] = cutPlaneNorm.GetY();
            this->torusInCuttingPlaneArray[cntRS][i*3+2] = cutPlaneNorm.GetZ();
            // torus center
            this->torusVertexArray[cntRS][i*3+0] = this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusCenter().GetX();
            this->torusVertexArray[cntRS][i*3+1] = this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusCenter().GetY();
            this->torusVertexArray[cntRS][i*3+2] = this->reducedSurface[cntRS]->GetRSEdge( i)->GetTorusCenter().GetZ();
        }
        
        ///////////////////////////////////////////////////////////
        // compute arrays for ray casting the spheres on the GPU //
        ///////////////////////////////////////////////////////////    

        this->sphereVertexArray[cntRS].AssertCapacity(
                this->reducedSurface[cntRS]->GetRSVertexCount() * 4);
        this->sphereVertexArray[cntRS].Clear();
        this->sphereColors[cntRS].AssertCapacity(
                this->reducedSurface[cntRS]->GetRSVertexCount() * 3);
        this->sphereColors[cntRS].Clear();
        this->sphereTexCoord[cntRS].AssertCapacity(
                this->reducedSurface[cntRS]->GetRSVertexCount() * 3);
        this->sphereTexCoord[cntRS].Clear();
        this->sphereSurfVector[cntRS].AssertCapacity(
                this->reducedSurface[cntRS]->GetRSVertexCount() * 4);
        this->sphereSurfVector[cntRS].Clear();

        // variables for surface vector calculation
        vislib::math::Vector<float, 3> vertexPos, surfVector, tempVec, edgeVector;
        vislib::Array<vislib::math::Vector<float, 3> > vecList;
        float maxDist, dist, atomRadius;
        unsigned int j, edgeCount;

        // loop over all RS-vertices (i.e. all protein atoms)
        for( i = 0; i < this->reducedSurface[cntRS]->GetRSVertexCount(); ++i ) {
            // add only surface atoms (i.e. with not buried RS-vertices)
            if( this->reducedSurface[cntRS]->GetRSVertex( i)->IsBuried() )
                continue;
            // set vertex color
            this->sphereColors[cntRS].Append( this->atomColor[this->reducedSurface[cntRS]->GetRSVertex( i)->GetIndex()].GetX());
            this->sphereColors[cntRS].Append( this->atomColor[this->reducedSurface[cntRS]->GetRSVertex( i)->GetIndex()].GetY());
            this->sphereColors[cntRS].Append( this->atomColor[this->reducedSurface[cntRS]->GetRSVertex( i)->GetIndex()].GetZ());
            // set vertex position
            this->sphereVertexArray[cntRS].Append(
                    this->reducedSurface[cntRS]->GetRSVertex( i)->GetPosition().GetX());
            this->sphereVertexArray[cntRS].Append(
                    this->reducedSurface[cntRS]->GetRSVertex( i)->GetPosition().GetY());
            this->sphereVertexArray[cntRS].Append(
                    this->reducedSurface[cntRS]->GetRSVertex( i)->GetPosition().GetZ());
            if( this->drawSAS ){
                this->sphereVertexArray[cntRS].Append(
                    this->reducedSurface[cntRS]->GetRSVertex( i)->GetRadius() + this->probeRadius);
            } else {
                this->sphereVertexArray[cntRS].Append(
                    this->reducedSurface[cntRS]->GetRSVertex( i)->GetRadius());
            }
            // set number of edges and texture coordinates
            this->sphereTexCoord[cntRS].Append(
                (float)this->reducedSurface[cntRS]->GetRSVertex( i)->GetEdgeCount());
            this->sphereTexCoord[cntRS].Append(
                (float)this->reducedSurface[cntRS]->GetRSVertex( i)->GetTexCoordX());
            this->sphereTexCoord[cntRS].Append(
                (float)this->reducedSurface[cntRS]->GetRSVertex( i)->GetTexCoordY());
            
            // calculate and set surface vector and max distance for additonal sphere interior cliping
            vertexPos = this->reducedSurface[cntRS]->GetRSVertex(i)->GetPosition();
            atomRadius = this->reducedSurface[cntRS]->GetRSVertex(i)->GetRadius();
            edgeCount = this->reducedSurface[cntRS]->GetRSVertex(i)->GetEdgeCount();
            vecList.AssertCapacity(edgeCount);
            vecList.Clear();

            surfVector.Set(0.0f, 0.0f, 0.0f);
            edgeVector.Set(0.0f, 0.0f, 0.0f);
            // calculate intersections of the sphere with triangle patches.
            // the sum of these (normalised) intersections is the surfVector.
            // the sum of all (normalised) edges is the edgeVector.
            for(j = 0; j < edgeCount; ++j ) {
                tempVec = this->reducedSurface[cntRS]->GetRSVertex(i)->GetEdge(j)->GetFace1()->GetProbeCenter() - vertexPos;
                tempVec.Normalise();
                if(!(vecList.Contains(tempVec))) {  
                    vecList.Append(tempVec);
                    surfVector += tempVec;
                }
                tempVec = this->reducedSurface[cntRS]->GetRSVertex(i)->GetEdge(j)->GetFace2()->GetProbeCenter() - vertexPos;
                tempVec.Normalise();
                if(!(vecList.Contains(tempVec))) {      
                    vecList.Append(tempVec);
                    surfVector += tempVec;
                }
                tempVec = this->reducedSurface[cntRS]->GetRSVertex(i)->GetEdge(j)->GetTorusCenter() - vertexPos;
                tempVec.Normalise();
                edgeVector += tempVec;
            }

            // if max angle of visible surface part is less than PI use surfVector else edgeVector
            if(edgeVector.Dot(edgeVector) > surfVector.Dot(surfVector)) {
                // look if there are several unconnected surface parts
                unsigned int edgeCounter = 1;
                const ReducedSurface::RSEdge * firstEdge = this->reducedSurface[cntRS]->GetRSVertex(i)->GetEdge(0);
                const ReducedSurface::RSVertex * currentVertex = this->reducedSurface[cntRS]->GetRSVertex(i);
                ReducedSurface::RSEdge * lastEdge = this->reducedSurface[cntRS]->GetRSVertex(i)->GetEdge(0);
                ReducedSurface::RSFace * lastFace = this->reducedSurface[cntRS]->GetRSVertex(i)->GetEdge(0)->GetFace1();
                ReducedSurface::RSEdge * currentEdge;
                for(j = 1; j < edgeCount; ++j) {
                    currentEdge = lastFace->GetEdge1();
                    if(((currentEdge->GetVertex1() == currentVertex) || (currentEdge->GetVertex2() == currentVertex)) && (currentEdge != lastEdge)) {
                        if(currentEdge == firstEdge) {
                            break;
                        } else {
                            lastEdge = currentEdge;
                            if(currentEdge->GetFace1() != lastFace) {
                                lastFace = currentEdge->GetFace1();
                            } else {
                                lastFace = currentEdge->GetFace2();
                            }
                            edgeCounter++;
                            continue;
                        }
                    }
                    currentEdge = lastFace->GetEdge2();
                    if(((currentEdge->GetVertex1() == currentVertex) || (currentEdge->GetVertex2() == currentVertex)) && (currentEdge != lastEdge)) {
                        if(currentEdge == firstEdge) {
                            break;
                        } else {
                            lastEdge = currentEdge;
                            if(currentEdge->GetFace1() != lastFace) {
                                lastFace = currentEdge->GetFace1();
                            } else {
                                lastFace = currentEdge->GetFace2();
                            }
                            edgeCounter++;
                            continue;
                        }
                    }
                    currentEdge = lastFace->GetEdge3();
                    if(((currentEdge->GetVertex1() == currentVertex) || (currentEdge->GetVertex2() == currentVertex)) && (currentEdge != lastEdge)) {
                        if(currentEdge == firstEdge) {
                            break;
                        } else {
                            lastEdge = currentEdge;
                            if(currentEdge->GetFace1() != lastFace) {
                                lastFace = currentEdge->GetFace1();
                            } else {
                                lastFace = currentEdge->GetFace2();
                            }
                            edgeCounter++;
                            continue;
                        }
                    }
                }
                // if there is only one surface part choose edgeVector
                // else there are several surface parts for which the default surfVector is better
                if(edgeCount == edgeCounter) {
                    surfVector = edgeVector*(-1.0f);
                }
            }
            surfVector.Normalise();
            surfVector *= atomRadius;

            // calculate max distance to surfVector
			maxDist = 0.0f;
			for(j = 0; j < edgeCount; ++j) {
				tempVec = (vecList[j]*atomRadius) - surfVector;
				dist = tempVec.Dot(tempVec);
				if(dist > maxDist) { maxDist = dist; }
            }

            this->sphereSurfVector[cntRS].Append(surfVector.GetX());
            this->sphereSurfVector[cntRS].Append(surfVector.GetY());
            this->sphereSurfVector[cntRS].Append(surfVector.GetZ());
            this->sphereSurfVector[cntRS].Append(maxDist);
        }
    }

    // print the time of the computation
    //std::cout << "computation of arrays for GPU ray casting finished:" << ( double( clock() - t) / double( CLOCKS_PER_SEC) ) << std::endl;
}


/*
 * Compute the vertex and attribute arrays for the raycasting shaders
 * (spheres, spherical triangles & tori)
 */
void ProteinRendererBDP::ComputeRaycastingArrays( unsigned int idxRS)
{
    // do nothing if the given index is out of bounds
    if( idxRS > this->reducedSurface.size() )
        return;

    // check if all arrays have the correct size
    if( this->sphericTriaVertexArray.size() != this->reducedSurface.size() ||
        this->sphericTriaVec1.size() != this->reducedSurface.size() ||
        this->sphericTriaVec2.size() != this->reducedSurface.size() ||
        this->sphericTriaVec3.size() != this->reducedSurface.size() ||
        this->sphericTriaTexCoord1.size() != this->reducedSurface.size() ||
        this->sphericTriaTexCoord2.size() != this->reducedSurface.size() ||
        this->sphericTriaTexCoord3.size() != this->reducedSurface.size() ||
        this->sphericTriaColors.size() != this->reducedSurface.size() ||
        this->torusVertexArray.size() != this->reducedSurface.size() ||
        this->torusInParamArray.size() != this->reducedSurface.size() ||
        this->torusQuatCArray.size() != this->reducedSurface.size() ||
        this->torusInSphereArray.size() != this->reducedSurface.size() ||
        this->torusColors.size() != this->reducedSurface.size() ||
        this->torusInCuttingPlaneArray.size() != this->reducedSurface.size() ||
        this->sphereVertexArray.size() != this->reducedSurface.size() ||
        this->sphereColors.size() != this->reducedSurface.size() ||
        this->sphereTexCoord.size() != this->reducedSurface.size() || 
        this->sphereSurfVector.size() != this->reducedSurface.size())
    {
        // recompute everything if one of the arrays has the wrong size
        //ComputeRaycastingArrays();
        this->preComputationDone = false;
        return;
    }
    
    unsigned int i;
    
    // compute singulatity textures
    this->CreateSingularityTexture( idxRS);

    // compute cutting planes texture (needs outNormals!)
    this->CreateCutPlanesTexture( idxRS);

    ///////////////////////////////////////////////////////////////////////
    // compute arrays for ray casting the spherical triangles on the GPU //
    ///////////////////////////////////////////////////////////////////////
    vislib::math::Vector<float, 3> tmpVec;
    vislib::math::Vector<float, 3> tmpDualProbe( 1.0f, 1.0f, 1.0f);
    float dualProbeRad = 0.0f;
    
    this->sphericTriaVertexArray[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSFaceCount() * 4);
    this->sphericTriaVec1[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSFaceCount() * 4);
    this->sphericTriaVec2[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSFaceCount() * 4);
    this->sphericTriaVec3[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSFaceCount() * 4);
    this->sphericTriaTexCoord1[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSFaceCount() * 3);
    this->sphericTriaTexCoord2[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSFaceCount() * 3);
    this->sphericTriaTexCoord3[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSFaceCount() * 3);
    this->sphericTriaColors[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSFaceCount() * 3);

    // loop over all RS-faces
    for( i = 0; i < this->reducedSurface[idxRS]->GetRSFaceCount(); ++i )
    {
        // if the face has a dual face --> store the probe of this face
        if( this->reducedSurface[idxRS]->GetRSFace( i)->GetDualFace() != NULL )
        {
            tmpDualProbe = this->reducedSurface[idxRS]->GetRSFace( i)->GetDualFace()->GetProbeCenter();
            dualProbeRad = this->probeRadius;
        }
        // first RS-vertex
        tmpVec = this->reducedSurface[idxRS]->GetRSFace( i)->GetVertex1()->GetPosition() - this->reducedSurface[idxRS]->GetRSFace( i)->GetProbeCenter();
        this->sphericTriaVec1[idxRS][i*4+0] = tmpVec.GetX();
        this->sphericTriaVec1[idxRS][i*4+1] = tmpVec.GetY();
        this->sphericTriaVec1[idxRS][i*4+2] = tmpVec.GetZ();
        this->sphericTriaVec1[idxRS][i*4+3] = 1.0f;
        // second RS-vertex
        tmpVec = this->reducedSurface[idxRS]->GetRSFace( i)->GetVertex2()->GetPosition() - this->reducedSurface[idxRS]->GetRSFace( i)->GetProbeCenter();
        this->sphericTriaVec2[idxRS][i*4+0] = tmpVec.GetX();
        this->sphericTriaVec2[idxRS][i*4+1] = tmpVec.GetY();
        this->sphericTriaVec2[idxRS][i*4+2] = tmpVec.GetZ();
        this->sphericTriaVec2[idxRS][i*4+3] = 1.0f;
        // third RS-vertex
        tmpVec = this->reducedSurface[idxRS]->GetRSFace( i)->GetVertex3()->GetPosition() - this->reducedSurface[idxRS]->GetRSFace( i)->GetProbeCenter();
        this->sphericTriaVec3[idxRS][i*4+0] = tmpVec.GetX();
        this->sphericTriaVec3[idxRS][i*4+1] = tmpVec.GetY();
        this->sphericTriaVec3[idxRS][i*4+2] = tmpVec.GetZ();
        this->sphericTriaVec3[idxRS][i*4+3] = dualProbeRad*dualProbeRad;
        // store number of cutting probes and texture coordinates for each edge
        this->sphericTriaTexCoord1[idxRS][i*3+0] = (float)this->reducedSurface[idxRS]->GetRSFace( i)->GetEdge1()->cuttingProbes.size();
        this->sphericTriaTexCoord1[idxRS][i*3+1] = (float)this->reducedSurface[idxRS]->GetRSFace( i)->GetEdge1()->GetTexCoordX();
        this->sphericTriaTexCoord1[idxRS][i*3+2] = (float)this->reducedSurface[idxRS]->GetRSFace( i)->GetEdge1()->GetTexCoordY();
        this->sphericTriaTexCoord2[idxRS][i*3+0] = (float)this->reducedSurface[idxRS]->GetRSFace( i)->GetEdge2()->cuttingProbes.size();
        this->sphericTriaTexCoord2[idxRS][i*3+1] = (float)this->reducedSurface[idxRS]->GetRSFace( i)->GetEdge2()->GetTexCoordX();
        this->sphericTriaTexCoord2[idxRS][i*3+2] = (float)this->reducedSurface[idxRS]->GetRSFace( i)->GetEdge2()->GetTexCoordY();
        this->sphericTriaTexCoord3[idxRS][i*3+0] = (float)this->reducedSurface[idxRS]->GetRSFace( i)->GetEdge3()->cuttingProbes.size();
        this->sphericTriaTexCoord3[idxRS][i*3+1] = (float)this->reducedSurface[idxRS]->GetRSFace( i)->GetEdge3()->GetTexCoordX();
        this->sphericTriaTexCoord3[idxRS][i*3+2] = (float)this->reducedSurface[idxRS]->GetRSFace( i)->GetEdge3()->GetTexCoordY();
        // colors
        this->sphericTriaColors[idxRS][i*3+0] = CodeColor( this->atomColor[this->reducedSurface[idxRS]->GetRSFace( i)->GetVertex1()->GetIndex()]);
        this->sphericTriaColors[idxRS][i*3+1] = CodeColor( this->atomColor[this->reducedSurface[idxRS]->GetRSFace( i)->GetVertex2()->GetIndex()]);
        this->sphericTriaColors[idxRS][i*3+2] = CodeColor( this->atomColor[this->reducedSurface[idxRS]->GetRSFace( i)->GetVertex3()->GetIndex()]);
        // sphere center
        this->sphericTriaVertexArray[idxRS][i*4+0] = this->reducedSurface[idxRS]->GetRSFace( i)->GetProbeCenter().GetX();
        this->sphericTriaVertexArray[idxRS][i*4+1] = this->reducedSurface[idxRS]->GetRSFace( i)->GetProbeCenter().GetY();
        this->sphericTriaVertexArray[idxRS][i*4+2] = this->reducedSurface[idxRS]->GetRSFace( i)->GetProbeCenter().GetZ();
        this->sphericTriaVertexArray[idxRS][i*4+3] = this->GetProbeRadius();
    }

    ////////////////////////////////////////////////////////
    // compute arrays for ray casting the tori on the GPU //
    ////////////////////////////////////////////////////////
    vislib::math::Quaternion<float> quatC;
    vislib::math::Vector<float, 3> zAxis, torusAxis, rotAxis, P, X1, X2, C, planeNormal;
    zAxis.Set( 0.0f, 0.0f, 1.0f);
    float distance, d;
    vislib::math::Vector<float, 3> tmpDir1, tmpDir2, tmpDir3, cutPlaneNorm;

    this->torusVertexArray[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSEdgeCount() * 3);
    this->torusInParamArray[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSEdgeCount() * 3);
    this->torusQuatCArray[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSEdgeCount() * 4);
    this->torusInSphereArray[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSEdgeCount() * 4);
    this->torusColors[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSEdgeCount() * 4);
    this->torusInCuttingPlaneArray[idxRS].SetCount( this->reducedSurface[idxRS]->GetRSEdgeCount() * 3);

    // loop over all RS-edges
    for( i = 0; i < this->reducedSurface[idxRS]->GetRSEdgeCount(); ++i )
    {
        // get the rotation axis of the torus
        torusAxis = this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusCenter();
        torusAxis.Normalise();
        // get the axis for rotating the torus rotations axis on the z-axis
        rotAxis = torusAxis.Cross( zAxis);
        rotAxis.Normalise();
        // compute quaternion
        quatC.Set( torusAxis.Angle( zAxis), rotAxis);
        // compute the tangential point X2 of the spheres
        P = this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusCenter() + rotAxis *
            this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusRadius();

        X1 = P - this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex1()->GetPosition();
        X1.Normalise();
        X1 *= this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex1()->GetRadius();
        X2 = P - this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex2()->GetPosition();
        X2.Normalise();
        X2 *= this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex2()->GetRadius();
        d = ( X1 + this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusCenter()).Dot( torusAxis);

        C = this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex1()->GetPosition() -
                this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex2()->GetPosition();
        C = ( ( P - this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex2()->GetPosition()).Length() /
                ( ( P - this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex1()->GetPosition()).Length() + 
                ( P - this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex2()->GetPosition()).Length())) * C;
        distance = ( X2 - C).Length();
        C = ( C + this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex2()->GetPosition()) -
                this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusCenter();
    
        // compute normal of the cutting plane
        tmpDir1 = this->reducedSurface[idxRS]->GetRSEdge( i)->GetFace1()->GetProbeCenter();
        tmpDir2 = this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex2()->GetPosition() - tmpDir1;
        tmpDir2.Normalise();
        tmpDir2 *= this->probeRadius;
        tmpDir2 = tmpDir2 + tmpDir1 - this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusCenter();
        tmpDir3 = this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - tmpDir1;
        tmpDir3.Normalise();
        tmpDir3 *= this->probeRadius;
        tmpDir3 = tmpDir3 + tmpDir1 - this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusCenter();
        // tmpDir2 and tmpDir3 now store the position of the intersection points for face 1
        cutPlaneNorm = tmpDir2 - tmpDir3;
        // cutPlaneNorm now stores the vector between the two intersection points for face 1
        tmpDir1 = this->reducedSurface[idxRS]->GetRSEdge( i)->GetFace2()->GetProbeCenter();
        tmpDir2 = this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - tmpDir1;
        tmpDir2.Normalise();
        tmpDir2 *= this->probeRadius;
        tmpDir2 = tmpDir2 + tmpDir1 - this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusCenter();
        // tmpDir2 now stores the position of the intersection point 1 for face 2
        tmpDir2 = tmpDir2 - tmpDir3;
        // tmpDir2 and tmpDir3 now span the plane containing the four intersection points
        cutPlaneNorm = cutPlaneNorm.Cross( tmpDir2);
        cutPlaneNorm = torusAxis.Cross( cutPlaneNorm);
        cutPlaneNorm.Normalise();
        
        // attributes
        this->torusInParamArray[idxRS][i*3+0] = this->probeRadius;
        this->torusInParamArray[idxRS][i*3+1] = this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusRadius();
        this->torusInParamArray[idxRS][i*3+2] = this->reducedSurface[idxRS]->GetRSEdge( i)->GetRotationAngle();
        this->torusQuatCArray[idxRS][i*4+0] = quatC.GetX();
        this->torusQuatCArray[idxRS][i*4+1] = quatC.GetY();
        this->torusQuatCArray[idxRS][i*4+2] = quatC.GetZ();
        this->torusQuatCArray[idxRS][i*4+3] = quatC.GetW();
        this->torusInSphereArray[idxRS][i*4+0] = C.GetX();
        this->torusInSphereArray[idxRS][i*4+1] = C.GetY();
        this->torusInSphereArray[idxRS][i*4+2] = C.GetZ();
        this->torusInSphereArray[idxRS][i*4+3] = distance;
        // colors
        this->torusColors[idxRS][i*4+0] = CodeColor( this->atomColor[this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex1()->GetIndex()]);
        this->torusColors[idxRS][i*4+1] = CodeColor( this->atomColor[this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex2()->GetIndex()]);
        this->torusColors[idxRS][i*4+2] = d;
        this->torusColors[idxRS][i*4+3] = ( X2 + this->reducedSurface[idxRS]->GetRSEdge( i)->GetVertex2()->GetPosition() - this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusCenter()).Dot( torusAxis) - d;
        // cutting plane
        this->torusInCuttingPlaneArray[idxRS][i*3+0] = cutPlaneNorm.GetX();
        this->torusInCuttingPlaneArray[idxRS][i*3+1] = cutPlaneNorm.GetY();
        this->torusInCuttingPlaneArray[idxRS][i*3+2] = cutPlaneNorm.GetZ();
        // torus center
        this->torusVertexArray[idxRS][i*3+0] = this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusCenter().GetX();
        this->torusVertexArray[idxRS][i*3+1] = this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusCenter().GetY();
        this->torusVertexArray[idxRS][i*3+2] = this->reducedSurface[idxRS]->GetRSEdge( i)->GetTorusCenter().GetZ();
    }
    
    ///////////////////////////////////////////////////////////
    // compute arrays for ray casting the spheres on the GPU //
    ///////////////////////////////////////////////////////////   
    this->sphereVertexArray[idxRS].AssertCapacity(
            this->reducedSurface[idxRS]->GetRSVertexCount() * 4);
    this->sphereVertexArray[idxRS].Clear();
    this->sphereColors[idxRS].AssertCapacity(
            this->reducedSurface[idxRS]->GetRSVertexCount() * 3);
    this->sphereColors[idxRS].Clear();
    this->sphereTexCoord[idxRS].AssertCapacity(
            this->reducedSurface[idxRS]->GetRSVertexCount() * 3);
    this->sphereTexCoord[idxRS].Clear();
    this->sphereSurfVector[idxRS].AssertCapacity(
            this->reducedSurface[idxRS]->GetRSVertexCount() * 4);
    this->sphereSurfVector[idxRS].Clear();

    // variables for surface vector calculation
    vislib::math::Vector<float, 3> edgeVector, vertexPos, surfVector, tempVec;
    vislib::Array<vislib::math::Vector<float, 3> > vecList;
    float maxDist, dist, atomRadius;
    unsigned int j, edgeCount;


    // loop over all RS-vertices (i.e. all protein atoms)
    for( i = 0; i < this->reducedSurface[idxRS]->GetRSVertexCount(); ++i )
    {
        // add only surface atoms (i.e. with not buried RS-vertices)
        if( this->reducedSurface[idxRS]->GetRSVertex( i)->IsBuried() )
            continue;
        // set vertex color
        this->sphereColors[idxRS].Append( this->atomColor[this->reducedSurface[idxRS]->GetRSVertex( i)->GetIndex()].GetX());
        this->sphereColors[idxRS].Append( this->atomColor[this->reducedSurface[idxRS]->GetRSVertex( i)->GetIndex()].GetY());
        this->sphereColors[idxRS].Append( this->atomColor[this->reducedSurface[idxRS]->GetRSVertex( i)->GetIndex()].GetZ());
        // set vertex position
        this->sphereVertexArray[idxRS].Append(
                this->reducedSurface[idxRS]->GetRSVertex( i)->GetPosition().GetX());
        this->sphereVertexArray[idxRS].Append(
                this->reducedSurface[idxRS]->GetRSVertex( i)->GetPosition().GetY());
        this->sphereVertexArray[idxRS].Append(
                this->reducedSurface[idxRS]->GetRSVertex( i)->GetPosition().GetZ());
        if( this->drawSAS ){
            this->sphereVertexArray[idxRS].Append(
                this->reducedSurface[idxRS]->GetRSVertex( i)->GetRadius() + this->probeRadius);
        } else {
            this->sphereVertexArray[idxRS].Append(
                this->reducedSurface[idxRS]->GetRSVertex( i)->GetRadius());
        }
        // set number of edges and texture coordinates
        this->sphereTexCoord[idxRS].Append(
            (float)this->reducedSurface[idxRS]->GetRSVertex( i)->GetEdgeCount());
        this->sphereTexCoord[idxRS].Append(
            (float)this->reducedSurface[idxRS]->GetRSVertex( i)->GetTexCoordX());
        this->sphereTexCoord[idxRS].Append(
            (float)this->reducedSurface[idxRS]->GetRSVertex( i)->GetTexCoordY());

        // calculate and set surface vector and max distance for additonal sphere interior cliping
        vertexPos = this->reducedSurface[idxRS]->GetRSVertex(i)->GetPosition();
        atomRadius = this->reducedSurface[idxRS]->GetRSVertex(i)->GetRadius();
        edgeCount = this->reducedSurface[idxRS]->GetRSVertex(i)->GetEdgeCount();
        vecList.AssertCapacity(edgeCount);
        vecList.Clear();

        surfVector.Set(0.0f, 0.0f, 0.0f);
        edgeVector.Set(0.0f, 0.0f, 0.0f);
        // calculate intersections of the sphere with triangle patches.
        // the sum of these (normalised) intersections is the surfVector.
        // the sum of all (normalised) edges is the edgeVector.
        for(j = 0; j < edgeCount; ++j ) {
            tempVec = this->reducedSurface[idxRS]->GetRSVertex(i)->GetEdge(j)->GetFace1()->GetProbeCenter() - vertexPos;
            tempVec.Normalise();
            if(!(vecList.Contains(tempVec))) {  
                vecList.Append(tempVec);
                surfVector += tempVec;
            }
            tempVec = this->reducedSurface[idxRS]->GetRSVertex(i)->GetEdge(j)->GetFace2()->GetProbeCenter() - vertexPos;
            tempVec.Normalise();
            if(!(vecList.Contains(tempVec))) {      
                vecList.Append(tempVec);
                surfVector += tempVec;
            }
            tempVec = this->reducedSurface[idxRS]->GetRSVertex(i)->GetEdge(j)->GetTorusCenter() - vertexPos;
            tempVec.Normalise();
            edgeVector += tempVec;
        }

        // if max angle of visible surface part is less than PI use surfVector else edgeVector
        if(edgeVector.Dot(edgeVector) > surfVector.Dot(surfVector)) {
            // look if there are several unconnected surface parts
            unsigned int edgeCounter = 1;
            const ReducedSurface::RSEdge * firstEdge = this->reducedSurface[idxRS]->GetRSVertex(i)->GetEdge(0);
            const ReducedSurface::RSVertex * currentVertex = this->reducedSurface[idxRS]->GetRSVertex(i);
            ReducedSurface::RSEdge * lastEdge = this->reducedSurface[idxRS]->GetRSVertex(i)->GetEdge(0);
            ReducedSurface::RSFace * lastFace = this->reducedSurface[idxRS]->GetRSVertex(i)->GetEdge(0)->GetFace1();
            ReducedSurface::RSEdge * currentEdge;
            for(j = 1; j < edgeCount; ++j) {
                currentEdge = lastFace->GetEdge1();
                if(((currentEdge->GetVertex1() == currentVertex) || (currentEdge->GetVertex2() == currentVertex)) && (currentEdge != lastEdge)) {
                    if(currentEdge == firstEdge) {
                        break;
                    } else {
                        lastEdge = currentEdge;
                        if(currentEdge->GetFace1() != lastFace) {
                            lastFace = currentEdge->GetFace1();
                        } else {
                            lastFace = currentEdge->GetFace2();
                        }
                        edgeCounter++;
                        continue;
                    }
                }
                currentEdge = lastFace->GetEdge2();
                if(((currentEdge->GetVertex1() == currentVertex) || (currentEdge->GetVertex2() == currentVertex)) && (currentEdge != lastEdge)) {
                    if(currentEdge == firstEdge) {
                        break;
                    } else {
                        lastEdge = currentEdge;
                        if(currentEdge->GetFace1() != lastFace) {
                            lastFace = currentEdge->GetFace1();
                        } else {
                            lastFace = currentEdge->GetFace2();
                        }
                        edgeCounter++;
                        continue;
                    }
                }
                currentEdge = lastFace->GetEdge3();
                if(((currentEdge->GetVertex1() == currentVertex) || (currentEdge->GetVertex2() == currentVertex)) && (currentEdge != lastEdge)) {
                    if(currentEdge == firstEdge) {
                        break;
                    } else {
                        lastEdge = currentEdge;
                        if(currentEdge->GetFace1() != lastFace) {
                            lastFace = currentEdge->GetFace1();
                        } else {
                            lastFace = currentEdge->GetFace2();
                        }
                        edgeCounter++;
                        continue;
                    }
                }
            }
            // if there is only one surface part choose edgeVector
            // else there are several surface parts for which the default surfVector is better
            if(edgeCount == edgeCounter) {
                surfVector = edgeVector*(-1.0f);
            }
        }
        surfVector.Normalise();
        surfVector *= atomRadius;

        // calculate max distance to surfVector
		maxDist = 0.0f;
		for(j = 0; j < edgeCount; ++j) {
			tempVec = (vecList[j]*atomRadius) - surfVector;
			dist = tempVec.Dot(tempVec);
			if(dist > maxDist) { maxDist = dist; }
		}
    
        this->sphereSurfVector[idxRS].Append(surfVector.GetX());
        this->sphereSurfVector[idxRS].Append(surfVector.GetY());
        this->sphereSurfVector[idxRS].Append(surfVector.GetZ());
        this->sphereSurfVector[idxRS].Append(maxDist);
    }
}


/*
 * Compute all vertex, attribute and color arrays used for ray casting 
 * the simplified molecular surface.
 */
void ProteinRendererBDP::ComputeRaycastingArraysSimple()
{
    //time_t t = clock();

    unsigned int cntRS;
    unsigned int i;
    //vislib::math::Vector<float, 3> col( 0.45f, 0.75f, 0.15f);
    //float codedcol = this->CodeColor( col);
    
    // resize lists of vertex, attribute and color arrays
    this->sphericTriaVertexArray.resize( this->simpleRS.size());
    this->sphericTriaVec1.resize( this->simpleRS.size());
    this->sphericTriaVec2.resize( this->simpleRS.size());
    this->sphericTriaVec3.resize( this->simpleRS.size());
    this->sphericTriaTexCoord1.resize( this->simpleRS.size());
    this->sphericTriaTexCoord2.resize( this->simpleRS.size());
    this->sphericTriaTexCoord3.resize( this->simpleRS.size());
    this->sphericTriaColors.resize( this->simpleRS.size());
    this->torusVertexArray.resize( this->simpleRS.size());
    this->torusInParamArray.resize( this->simpleRS.size());
    this->torusQuatCArray.resize( this->simpleRS.size());
    this->torusInSphereArray.resize( this->simpleRS.size());
    this->torusColors.resize( this->simpleRS.size());
    this->torusInCuttingPlaneArray.resize( this->simpleRS.size());
    this->sphereVertexArray.resize( this->simpleRS.size());
    this->sphereColors.resize( this->simpleRS.size());
    
    
    // compute singulatity textures
    this->CreateSingularityTexturesSimple();
    
    for( cntRS = 0; cntRS < this->simpleRS.size(); ++cntRS )
    {
        ///////////////////////////////////////////////////////////////////////
        // compute arrays for ray casting the spherical triangles on the GPU //
        ///////////////////////////////////////////////////////////////////////
        vislib::math::Vector<float, 3> tmpVec;
        vislib::math::Vector<float, 3> tmpDualProbe( 1.0f, 1.0f, 1.0f);
        float dualProbeRad = 0.0f;
        
        this->sphericTriaVertexArray[cntRS].SetCount( this->simpleRS[cntRS]->GetRSFaceCount() * 4);
        this->sphericTriaVec1[cntRS].SetCount( this->simpleRS[cntRS]->GetRSFaceCount() * 4);
        this->sphericTriaVec2[cntRS].SetCount( this->simpleRS[cntRS]->GetRSFaceCount() * 4);
        this->sphericTriaVec3[cntRS].SetCount( this->simpleRS[cntRS]->GetRSFaceCount() * 4);
        this->sphericTriaTexCoord1[cntRS].SetCount( this->simpleRS[cntRS]->GetRSFaceCount() * 3);
        this->sphericTriaTexCoord2[cntRS].SetCount( this->simpleRS[cntRS]->GetRSFaceCount() * 3);
        this->sphericTriaTexCoord3[cntRS].SetCount( this->simpleRS[cntRS]->GetRSFaceCount() * 3);
        this->sphericTriaColors[cntRS].SetCount( this->simpleRS[cntRS]->GetRSFaceCount() * 3);
    
        // loop over all RS-faces
        for( i = 0; i < this->simpleRS[cntRS]->GetRSFaceCount(); ++i )
        {
            // if the face has a dual face --> store the probe of this face
            if( this->simpleRS[cntRS]->GetRSFace( i)->GetDualFace() != NULL )
            {
                tmpDualProbe = this->simpleRS[cntRS]->GetRSFace( i)->GetDualFace()->GetProbeCenter();
                dualProbeRad = this->probeRadius;
            }
            // first RS-vertex
            tmpVec = this->simpleRS[cntRS]->GetRSFace( i)->GetVertex1()->GetPosition() - this->simpleRS[cntRS]->GetRSFace( i)->GetProbeCenter();
            this->sphericTriaVec1[cntRS][i*4+0] = tmpVec.GetX();
            this->sphericTriaVec1[cntRS][i*4+1] = tmpVec.GetY();
            this->sphericTriaVec1[cntRS][i*4+2] = tmpVec.GetZ();
            this->sphericTriaVec1[cntRS][i*4+3] = 1.0f;
            // second RS-vertex
            tmpVec = this->simpleRS[cntRS]->GetRSFace( i)->GetVertex2()->GetPosition() - this->simpleRS[cntRS]->GetRSFace( i)->GetProbeCenter();
            this->sphericTriaVec2[cntRS][i*4+0] = tmpVec.GetX();
            this->sphericTriaVec2[cntRS][i*4+1] = tmpVec.GetY();
            this->sphericTriaVec2[cntRS][i*4+2] = tmpVec.GetZ();
            this->sphericTriaVec2[cntRS][i*4+3] = 1.0f;
            // third RS-vertex
            tmpVec = this->simpleRS[cntRS]->GetRSFace( i)->GetVertex3()->GetPosition() - this->simpleRS[cntRS]->GetRSFace( i)->GetProbeCenter();
            this->sphericTriaVec3[cntRS][i*4+0] = tmpVec.GetX();
            this->sphericTriaVec3[cntRS][i*4+1] = tmpVec.GetY();
            this->sphericTriaVec3[cntRS][i*4+2] = tmpVec.GetZ();
            this->sphericTriaVec3[cntRS][i*4+3] = dualProbeRad*dualProbeRad;
            // store number of cutting probes and texture coordinates for each edge
            this->sphericTriaTexCoord1[cntRS][i*3+0] = (float)this->simpleRS[cntRS]->GetRSFace( i)->GetEdge1()->cuttingProbes.size();
            this->sphericTriaTexCoord1[cntRS][i*3+1] = (float)this->simpleRS[cntRS]->GetRSFace( i)->GetEdge1()->GetTexCoordX();
            this->sphericTriaTexCoord1[cntRS][i*3+2] = (float)this->simpleRS[cntRS]->GetRSFace( i)->GetEdge1()->GetTexCoordY();
            this->sphericTriaTexCoord2[cntRS][i*3+0] = (float)this->simpleRS[cntRS]->GetRSFace( i)->GetEdge2()->cuttingProbes.size();
            this->sphericTriaTexCoord2[cntRS][i*3+1] = (float)this->simpleRS[cntRS]->GetRSFace( i)->GetEdge2()->GetTexCoordX();
            this->sphericTriaTexCoord2[cntRS][i*3+2] = (float)this->simpleRS[cntRS]->GetRSFace( i)->GetEdge2()->GetTexCoordY();
            this->sphericTriaTexCoord3[cntRS][i*3+0] = (float)this->simpleRS[cntRS]->GetRSFace( i)->GetEdge3()->cuttingProbes.size();
            this->sphericTriaTexCoord3[cntRS][i*3+1] = (float)this->simpleRS[cntRS]->GetRSFace( i)->GetEdge3()->GetTexCoordX();
            this->sphericTriaTexCoord3[cntRS][i*3+2] = (float)this->simpleRS[cntRS]->GetRSFace( i)->GetEdge3()->GetTexCoordY();
            // colors
            //this->sphericTriaColors[cntRS][i*3+0] = 125255.0f;
            //this->sphericTriaColors[cntRS][i*3+1] = 125255.0f;
            //this->sphericTriaColors[cntRS][i*3+2] = 125255.0f;
            //this->sphericTriaColors[cntRS][i*3+0] = codedcol;
            //this->sphericTriaColors[cntRS][i*3+1] = codedcol;
            //this->sphericTriaColors[cntRS][i*3+2] = codedcol;
            this->sphericTriaColors[cntRS][i*3+0] = CodeColor( this->GetProteinAtomColor( this->simpleRS[cntRS]->GetRSFace( i)->GetVertex1()->GetIndex()));
            this->sphericTriaColors[cntRS][i*3+1] = CodeColor( this->GetProteinAtomColor( this->simpleRS[cntRS]->GetRSFace( i)->GetVertex2()->GetIndex()));
            this->sphericTriaColors[cntRS][i*3+2] = CodeColor( this->GetProteinAtomColor( this->simpleRS[cntRS]->GetRSFace( i)->GetVertex3()->GetIndex()));
            // sphere center
            this->sphericTriaVertexArray[cntRS][i*4+0] = this->simpleRS[cntRS]->GetRSFace( i)->GetProbeCenter().GetX();
            this->sphericTriaVertexArray[cntRS][i*4+1] = this->simpleRS[cntRS]->GetRSFace( i)->GetProbeCenter().GetY();
            this->sphericTriaVertexArray[cntRS][i*4+2] = this->simpleRS[cntRS]->GetRSFace( i)->GetProbeCenter().GetZ();
            this->sphericTriaVertexArray[cntRS][i*4+3] = this->GetProbeRadius();
        }
    
        ////////////////////////////////////////////////////////
        // compute arrays for ray casting the tori on the GPU //
        ////////////////////////////////////////////////////////
        vislib::math::Quaternion<float> quatC;
        vislib::math::Vector<float, 3> zAxis, torusAxis, rotAxis, P, X1, X2, C, planeNormal;
        zAxis.Set( 0.0f, 0.0f, 1.0f);
        float distance, d;
        vislib::math::Vector<float, 3> tmpDir1, tmpDir2, tmpDir3, cutPlaneNorm;
    
        this->torusVertexArray[cntRS].SetCount( this->simpleRS[cntRS]->GetRSEdgeCount() * 3);
        this->torusInParamArray[cntRS].SetCount( this->simpleRS[cntRS]->GetRSEdgeCount() * 3);
        this->torusQuatCArray[cntRS].SetCount( this->simpleRS[cntRS]->GetRSEdgeCount() * 4);
        this->torusInSphereArray[cntRS].SetCount( this->simpleRS[cntRS]->GetRSEdgeCount() * 4);
        this->torusColors[cntRS].SetCount( this->simpleRS[cntRS]->GetRSEdgeCount() * 4);
        this->torusInCuttingPlaneArray[cntRS].SetCount( this->simpleRS[cntRS]->GetRSEdgeCount() * 3);
    
        // loop over all RS-edges
        for( i = 0; i < this->simpleRS[cntRS]->GetRSEdgeCount(); ++i )
        {
            // get the rotation axis of the torus
            torusAxis = this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusCenter();
            torusAxis.Normalise();
            // get the axis for rotating the torus rotations axis on the z-axis
            rotAxis = torusAxis.Cross( zAxis);
            rotAxis.Normalise();
            // compute quaternion
            quatC.Set( torusAxis.Angle( zAxis), rotAxis);
            // compute the tangential point X2 of the spheres
            P = this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusCenter() + rotAxis *
                this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusRadius();
    
            X1 = P - this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition();
            X1.Normalise();
            X1 *= this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex1()->GetRadius();
            X2 = P - this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition();
            X2.Normalise();
            X2 *= this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex2()->GetRadius();
            d = ( X1 + this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusCenter()).Dot( torusAxis);
    
            C = this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition() -
                    this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition();
            C = ( ( P - this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition()).Length() /
                    ( ( P - this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition()).Length() + 
                    ( P - this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition()).Length())) * C;
            distance = ( X2 - C).Length();
            C = ( C + this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition()) -
                    this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusCenter();
            
            // compute normal of the cutting plane
            tmpDir1 = this->simpleRS[cntRS]->GetRSEdge( i)->GetFace1()->GetProbeCenter();
            tmpDir2 = this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition() - tmpDir1;
            tmpDir2.Normalise();
            tmpDir2 *= this->probeRadius;
            tmpDir2 = tmpDir2 + tmpDir1 - this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusCenter();
            tmpDir3 = this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - tmpDir1;
            tmpDir3.Normalise();
            tmpDir3 *= this->probeRadius;
            tmpDir3 = tmpDir3 + tmpDir1 - this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusCenter();
            // tmpDir2 and tmpDir3 now store the position of the intersection points for face 1
            cutPlaneNorm = tmpDir2 - tmpDir3;
            // cutPlaneNorm now stores the vector between the two intersection points for face 1
            tmpDir1 = this->simpleRS[cntRS]->GetRSEdge( i)->GetFace2()->GetProbeCenter();
            tmpDir2 = this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex1()->GetPosition() - tmpDir1;
            tmpDir2.Normalise();
            tmpDir2 *= this->probeRadius;
            tmpDir2 = tmpDir2 + tmpDir1 - this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusCenter();
            // tmpDir2 now stores the position of the intersection point 1 for face 2
            tmpDir2 = tmpDir2 - tmpDir3;
            // tmpDir2 and tmpDir3 now span the plane containing the four intersection points
            cutPlaneNorm = cutPlaneNorm.Cross( tmpDir2);
            cutPlaneNorm = torusAxis.Cross( cutPlaneNorm);
            cutPlaneNorm.Normalise();
            
            // attributes
            this->torusInParamArray[cntRS][i*3+0] = this->probeRadius;
            this->torusInParamArray[cntRS][i*3+1] = this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusRadius();
            this->torusInParamArray[cntRS][i*3+2] = this->simpleRS[cntRS]->GetRSEdge( i)->GetRotationAngle();
            this->torusQuatCArray[cntRS][i*4+0] = quatC.GetX();
            this->torusQuatCArray[cntRS][i*4+1] = quatC.GetY();
            this->torusQuatCArray[cntRS][i*4+2] = quatC.GetZ();
            this->torusQuatCArray[cntRS][i*4+3] = quatC.GetW();
            this->torusInSphereArray[cntRS][i*4+0] = C.GetX();
            this->torusInSphereArray[cntRS][i*4+1] = C.GetY();
            this->torusInSphereArray[cntRS][i*4+2] = C.GetZ();
            this->torusInSphereArray[cntRS][i*4+3] = distance;
            // colors
            //this->torusColors[cntRS][i*4+0] = 250120000.0f;
            //this->torusColors[cntRS][i*4+1] = 250120000.0f;
            //this->torusColors[cntRS][i*4+0] = codedcol;
            //this->torusColors[cntRS][i*4+1] = codedcol;
            this->torusColors[cntRS][i*4+0] = CodeColor( this->GetProteinAtomColor( this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex1()->GetIndex()));
            this->torusColors[cntRS][i*4+1] = CodeColor( this->GetProteinAtomColor( this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex2()->GetIndex()));
            this->torusColors[cntRS][i*4+2] = d;
            //this->torusColors[cntRS][i*4+3] = ( X2 - X1).Length();
            this->torusColors[cntRS][i*4+3] = ( X2 + this->simpleRS[cntRS]->GetRSEdge( i)->GetVertex2()->GetPosition() - this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusCenter()).Dot( torusAxis) - d;
            // cutting plane
            this->torusInCuttingPlaneArray[cntRS][i*3+0] = cutPlaneNorm.GetX();
            this->torusInCuttingPlaneArray[cntRS][i*3+1] = cutPlaneNorm.GetY();
            this->torusInCuttingPlaneArray[cntRS][i*3+2] = cutPlaneNorm.GetZ();
            // torus center
            this->torusVertexArray[cntRS][i*3+0] = this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusCenter().GetX();
            this->torusVertexArray[cntRS][i*3+1] = this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusCenter().GetY();
            this->torusVertexArray[cntRS][i*3+2] = this->simpleRS[cntRS]->GetRSEdge( i)->GetTorusCenter().GetZ();
        }
        
        ///////////////////////////////////////////////////////////
        // compute arrays for ray casting the spheres on the GPU //
        ///////////////////////////////////////////////////////////    
        /*
        this->sphereVertexArray[cntRS].SetCount( this->simpleRS[cntRS]->GetRSVertexCount() * 4);
        this->sphereColors[cntRS].SetCount( this->simpleRS[cntRS]->GetRSVertexCount() * 3);
        */
        this->sphereVertexArray[cntRS].AssertCapacity(
                this->simpleRS[cntRS]->GetRSVertexCount() * 4);
        this->sphereVertexArray[cntRS].Clear();
        this->sphereColors[cntRS].AssertCapacity(
                this->simpleRS[cntRS]->GetRSVertexCount() * 3);
        this->sphereColors[cntRS].Clear();
        
        // loop over all RS-vertices (i.e. all protein atoms)
        for( i = 0; i < this->simpleRS[cntRS]->GetRSVertexCount(); ++i )
        {
            // add only surface atoms (i.e. with not buried RS-vertices)
            //if( this->simpleRS[cntRS]->GetRSVertex( i)->IsBuried() )
            //    continue;
            // set vertex color
            //this->sphereColors[cntRS].Append( col.GetX());
            //this->sphereColors[cntRS].Append( col.GetY());
            //this->sphereColors[cntRS].Append( col.GetZ());
            this->sphereColors[cntRS].Append( this->GetProteinAtomColor( this->simpleRS[cntRS]->GetRSVertex( i)->GetIndex()).GetX());
            this->sphereColors[cntRS].Append( this->GetProteinAtomColor( this->simpleRS[cntRS]->GetRSVertex( i)->GetIndex()).GetY());
            this->sphereColors[cntRS].Append( this->GetProteinAtomColor( this->simpleRS[cntRS]->GetRSVertex( i)->GetIndex()).GetZ());
            // set vertex position
            this->sphereVertexArray[cntRS].Append(
                    this->simpleRS[cntRS]->GetRSVertex( i)->GetPosition().GetX());
            this->sphereVertexArray[cntRS].Append(
                    this->simpleRS[cntRS]->GetRSVertex( i)->GetPosition().GetY());
            this->sphereVertexArray[cntRS].Append(
                    this->simpleRS[cntRS]->GetRSVertex( i)->GetPosition().GetZ());
            this->sphereVertexArray[cntRS].Append(
                    this->simpleRS[cntRS]->GetRSVertex( i)->GetRadius());
        }
    }
    // print the time of the computation
    //std::cout << "computation of arrays for GPU ray casting finished:" << ( double( clock() - t) / double( CLOCKS_PER_SEC) ) << std::endl;
}


/*
 * code a rgb-color into one float
 */
float ProteinRendererBDP::CodeColor( const vislib::math::Vector<float, 3> &col) const
{
    return float(
          (int)( col.GetX() * 255.0f)*1000000    // red
        + (int)( col.GetY() * 255.0f)*1000        // green
        + (int)( col.GetZ() * 255.0f) );            // blue
}


/*
 * decode a coded color to the original rgb-color
 */
vislib::math::Vector<float, 3> ProteinRendererBDP::DecodeColor( int codedColor) const
{
    int col = codedColor;
    vislib::math::Vector<float, 3> color;
    float red, green;
    if( col >= 1000000 )
        red = floor( (float)col / 1000000.0f);
    else
        red = 0.0;
    col = col - int( red * 1000000.0f);
    if( col > 1000)
        green = floor( (float)col / 1000.0f);
    else
        green = 0.0;
    col = col - int( green * 1000.0f);
    //color.Set( red / 255.0f, green / 255.0f, float(col) / 255.0f);
    color.Set( std::min( 1.0f, std::max( 0.0f, red / 255.0f ) ),
                  std::min( 1.0f, std::max( 0.0f, green / 255.0f ) ),
                  std::min( 1.0f, std::max( 0.0f, col / 255.0f ) ) );
    return color;
}


/*
 * Creates the cutting planes texture for sphere interior clipping.
 */
void ProteinRendererBDP::CreateCutPlanesTextures()
{
/*
    time_t t = clock();
*/
    unsigned int cnt1, cnt2, cntRS;
    
    // delete old cutting planes textures
    for( cnt1 = 0; cnt1 < this->cutPlanesTexture.size(); ++cnt1 )
    {
        glDeleteTextures( 1, &this->cutPlanesTexture[cnt1]);
    }
    // check if the cutting planes texture has the right size
    if( this->reducedSurface.size() != this->cutPlanesTexture.size() )
    {
        // store old cutting planes texture size
        unsigned int cutPlanesTexSizeOld = (unsigned int)this->cutPlanesTexture.size();
        // resize cutting planes texture to fit the number of reduced surfaces
        this->cutPlanesTexture.resize( this->reducedSurface.size());
        // generate a new texture for each new cutting planes texture
        for( cnt1 = cutPlanesTexSizeOld; cnt1 < this->cutPlanesTexture.size(); ++cnt1 )
        {
            glGenTextures( 1, &this->cutPlanesTexture[cnt1]);
        }
    }
    // resize cutting planes texture dimension and data arrays
    this->cutPlanesTexWidth.resize( this->reducedSurface.size());
    this->cutPlanesTexHeight.resize( this->reducedSurface.size());
    this->cutPlanesTexData.resize( this->reducedSurface.size());
    
    // get maximum texture size
    GLint texSize;
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &texSize);

    // TODO: compute proper number of maximum edges per vertex
    unsigned int maxNumEdges = 16;

    unsigned int coordX;
    unsigned int coordY;
    unsigned int edgeCount;
    vislib::math::Vector<float, 3> X1, zAxis, rotAxis, edgeVector, vertexPos, final;
    float atomRadius;
    zAxis.Set( 0.0f, 0.0f, 1.0f);

    for( cntRS = 0; cntRS < this->reducedSurface.size(); ++cntRS )
    {
        // set width and height of texture
        if( (unsigned int)texSize < this->reducedSurface[cntRS]->GetRSVertexCount() )
        {
            this->cutPlanesTexHeight[cntRS] = texSize;
            this->cutPlanesTexWidth[cntRS] = maxNumEdges * (int)ceil( double(
                this->reducedSurface[cntRS]->GetRSVertexCount()) / (double)texSize);
        }
        else
        {
            this->cutPlanesTexHeight[cntRS] = this->reducedSurface[cntRS]->GetRSVertexCount();
            this->cutPlanesTexWidth[cntRS] = maxNumEdges;
        }
        // generate float-array for texture with the appropriate dimension
        if( !this->cutPlanesTexData[cntRS].IsEmpty() )
            this->cutPlanesTexData[cntRS].Clear(true);
        this->cutPlanesTexData[cntRS].AssertCapacity(this->cutPlanesTexWidth[cntRS]*this->cutPlanesTexHeight[cntRS]*3);

        coordX = 0;
        coordY = 0;
        for( cnt1 = 0; cnt1 < this->reducedSurface[cntRS]->GetRSVertexCount(); ++cnt1 ) {
            if( this->reducedSurface[cntRS]->GetRSVertex( cnt1)->IsBuried() ) { continue; }

            this->reducedSurface[cntRS]->GetRSVertex( cnt1)->SetTexCoord( coordX, coordY);
            coordY++;
            if( coordY == this->cutPlanesTexHeight[cntRS] ) {
                coordY = 0;
                coordX = coordX + maxNumEdges;
            }

            edgeCount = this->reducedSurface[cntRS]->GetRSVertex( cnt1)->GetEdgeCount();
            vertexPos = this->reducedSurface[cntRS]->GetRSVertex( cnt1)->GetPosition();
            atomRadius = this->reducedSurface[cntRS]->GetRSVertex( cnt1)->GetRadius();

            for( cnt2 = 0; cnt2 < maxNumEdges; ++cnt2 ) {
                if(cnt2 < edgeCount) {
                    ReducedSurface::RSEdge* currentEdge = this->reducedSurface[cntRS]->GetRSVertex( cnt1)->GetEdge(cnt2);
                    edgeVector = currentEdge->GetTorusCenter() - vertexPos;
                    rotAxis = edgeVector.Cross( zAxis);
                    rotAxis.Normalise();
                    X1 = edgeVector + rotAxis * currentEdge->GetTorusRadius();
                    X1.Normalise();
                    X1 *= atomRadius;
                    edgeVector.Normalise();
                    final = edgeVector * (edgeVector.Dot(X1));

                    this->cutPlanesTexData[cntRS].Append(final.GetX());
                    this->cutPlanesTexData[cntRS].Append(final.GetY());
                    this->cutPlanesTexData[cntRS].Append(final.GetZ());

                } else {
                    this->cutPlanesTexData[cntRS].Append(0.0f);
                    this->cutPlanesTexData[cntRS].Append(0.0f);
                    this->cutPlanesTexData[cntRS].Append(0.0f);
                }
            }
        }

        // texture generation
        glBindTexture( GL_TEXTURE_2D, this->cutPlanesTexture[cntRS]);
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, this->cutPlanesTexWidth[cntRS], this->cutPlanesTexHeight[cntRS], 0, GL_RGB, GL_FLOAT, 
                this->cutPlanesTexData[cntRS].PeekElements());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture( GL_TEXTURE_2D, 0);
    }
/*
    std::cout << "Create texture: " <<
        ( double( clock() - t) / double( CLOCKS_PER_SEC) ) << std::endl;
*/
}


/*
 * Creates the cutting planes texture for sphere interior clipping.
 */
void ProteinRendererBDP::CreateCutPlanesTexture( unsigned int idxRS)
{
    //vislib::sys::Log::DefaultLog.WriteMsg( vislib::sys::Log::LEVEL_WARN, "===== 1 create cut planes texture IDX \n" );

    // do nothing if the index is out of bounds
    if( idxRS > this->reducedSurface.size() )
        return;
    
    // check if all arrays have the appropriate size
    if( this->cutPlanesTexture.size() != this->reducedSurface.size() ||
        this->cutPlanesTexWidth.size() != this->reducedSurface.size() ||
        this->cutPlanesTexHeight.size() != this->reducedSurface.size() ||
        this->cutPlanesTexData.size() != this->reducedSurface.size())
    {
        // create all cutting plane textures (?)
        CreateCutPlanesTextures();
        return;
    }
    
    unsigned int cnt1, cnt2;
    
    // delete old cutting planes texture
    glDeleteTextures( 1, &this->cutPlanesTexture[idxRS]);
    
    // get maximum texture size
    GLint texSize;
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &texSize);

    // TODO: compute proper number of maximum edges per vertex
    unsigned int maxNumEdges = 16;

    // set width and height of texture
    if( (unsigned int)texSize < this->reducedSurface[idxRS]->GetRSVertexCount() )
    {
        this->cutPlanesTexHeight[idxRS] = texSize;
        this->cutPlanesTexWidth[idxRS] = maxNumEdges * (int)ceil( double(
            this->reducedSurface[idxRS]->GetRSVertexCount()) / (double)texSize);
    }
    else
    {
        this->cutPlanesTexHeight[idxRS] = this->reducedSurface[idxRS]->GetRSVertexCount();
        this->cutPlanesTexWidth[idxRS] = maxNumEdges;
    }
    // generate float-array for texture with the appropriate dimension
    if( !this->cutPlanesTexData[idxRS].IsEmpty() )
        this->cutPlanesTexData[idxRS].Clear(true);
    this->cutPlanesTexData[idxRS].AssertCapacity(this->cutPlanesTexWidth[idxRS]*this->cutPlanesTexHeight[idxRS]*3);
    
    unsigned int coordX = 0;
    unsigned int coordY = 0;
    unsigned int edgeCount;
    vislib::math::Vector<float, 3> X1, zAxis, rotAxis, edgeVector, vertexPos, final;
    float atomRadius;
    zAxis.Set( 0.0f, 0.0f, 1.0f);

    for( cnt1 = 0; cnt1 < this->reducedSurface[idxRS]->GetRSVertexCount(); ++cnt1 ) {
        if( this->reducedSurface[idxRS]->GetRSVertex( cnt1)->IsBuried() ) { continue; }

        this->reducedSurface[idxRS]->GetRSVertex( cnt1)->SetTexCoord( coordX, coordY);
        coordY++;
        if( coordY == this->cutPlanesTexHeight[idxRS] ) {
            coordY = 0;
            coordX = coordX + maxNumEdges;
        }

        edgeCount = this->reducedSurface[idxRS]->GetRSVertex( cnt1)->GetEdgeCount();
        vertexPos = this->reducedSurface[idxRS]->GetRSVertex( cnt1)->GetPosition();
        atomRadius = this->reducedSurface[idxRS]->GetRSVertex( cnt1)->GetRadius();

        for( cnt2 = 0; cnt2 < maxNumEdges; ++cnt2 ) {
            if(cnt2 < edgeCount) {
                ReducedSurface::RSEdge* currentEdge = this->reducedSurface[idxRS]->GetRSVertex( cnt1)->GetEdge(cnt2);
                edgeVector = currentEdge->GetTorusCenter() - vertexPos;
                rotAxis = edgeVector.Cross( zAxis);
                rotAxis.Normalise();
                X1 = edgeVector + rotAxis * currentEdge->GetTorusRadius();
                X1.Normalise();
                X1 *= atomRadius;
                edgeVector.Normalise();
                final = edgeVector * (edgeVector.Dot(X1));

                this->cutPlanesTexData[idxRS].Append(final.GetX());
                this->cutPlanesTexData[idxRS].Append(final.GetY());
                this->cutPlanesTexData[idxRS].Append(final.GetZ());
            } else {
                this->cutPlanesTexData[idxRS].Append(0.0f);
                this->cutPlanesTexData[idxRS].Append(0.0f);
                this->cutPlanesTexData[idxRS].Append(0.0f);
            }
        }
    }

    // texture generation
    glBindTexture( GL_TEXTURE_2D, this->cutPlanesTexture[idxRS]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, this->cutPlanesTexWidth[idxRS], this->cutPlanesTexHeight[idxRS], 0, GL_RGB, GL_FLOAT, 
            this->cutPlanesTexData[idxRS].PeekElements());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture( GL_TEXTURE_2D, 0);
}


/*
 * Creates the texture for singularity handling.
 */
void ProteinRendererBDP::CreateSingularityTextures()
{
/*
    time_t t = clock();
*/
    unsigned int cnt1, cnt2, cntRS;
    
    // delete old singularity textures
    for( cnt1 = 0; cnt1 < this->singularityTexture.size(); ++cnt1 )
    {
        glDeleteTextures( 1, &singularityTexture[cnt1]);
    }
    // check if the singularity texture has the right size
    if( this->reducedSurface.size() != this->singularityTexture.size() )
    {
        // store old singularity texture size
        unsigned int singTexSizeOld = (unsigned int)this->singularityTexture.size();
        // resize singularity texture to fit the number of reduced surfaces
        this->singularityTexture.resize( this->reducedSurface.size());
        // generate a new texture for each new singularity texture
        for( cnt1 = singTexSizeOld; cnt1 < singularityTexture.size(); ++cnt1 )
        {
            glGenTextures( 1, &singularityTexture[cnt1]);
        }
    }
    // resize singularity texture dimension arrays
    this->singTexWidth.resize( this->reducedSurface.size());
    this->singTexHeight.resize( this->reducedSurface.size());
    
    // get maximum texture size
    GLint texSize;
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &texSize);
        
    // TODO: compute proper maximum number of cutting probes
    unsigned int numProbes = 16;
    
    for( cntRS = 0; cntRS < this->reducedSurface.size(); ++cntRS )
    {
        // set width and height of texture
        if( (unsigned int)texSize < this->reducedSurface[cntRS]->GetCutRSEdgesCount() )
        {
            this->singTexHeight[cntRS] = texSize;
            this->singTexWidth[cntRS] = numProbes * (int)ceil( double(
                this->reducedSurface[cntRS]->GetCutRSEdgesCount()) / (double)texSize);
        }
        else
        {
            this->singTexHeight[cntRS] = this->reducedSurface[cntRS]->GetCutRSEdgesCount();
            this->singTexWidth[cntRS] = numProbes;
        }
        // generate float-array for texture with the appropriate dimension
        if( this->singTexData )
            delete[] this->singTexData;
        this->singTexData = new float[this->singTexWidth[cntRS]*this->singTexHeight[cntRS]*3];
        // write probes to singularity texture
        unsigned int coordX = 0;
        unsigned int coordY = 0;
        unsigned int counter = 0;
        for( cnt1 = 0; cnt1 < this->reducedSurface[cntRS]->GetRSEdgeCount(); ++cnt1 )
        {
            if( this->reducedSurface[cntRS]->GetRSEdge( cnt1)->cuttingProbes.empty() )
            {
                this->reducedSurface[cntRS]->GetRSEdge( cnt1)->SetTexCoord( 0, 0);
            }
            else
            {
                // set texture coordinates
                this->reducedSurface[cntRS]->GetRSEdge( cnt1)->SetTexCoord( coordX, coordY);
                // compute texture coordinates for next entry
                coordY++;
                if( coordY == this->singTexHeight[cntRS] )
                {
                    coordY = 0;
                    coordX = coordX + numProbes;
                }
                // write probes to texture
                for( cnt2 = 0; cnt2 < numProbes; ++cnt2 )
                {
                    if( cnt2 < this->reducedSurface[cntRS]->GetRSEdge( cnt1)->cuttingProbes.size() )
                    {
                        singTexData[counter] =
                                this->reducedSurface[cntRS]->GetRSEdge( cnt1)->cuttingProbes[cnt2]->GetProbeCenter().GetX();
                        counter++;
                        singTexData[counter] =
                                this->reducedSurface[cntRS]->GetRSEdge( cnt1)->cuttingProbes[cnt2]->GetProbeCenter().GetY();
                        counter++;
                        singTexData[counter] =
                                this->reducedSurface[cntRS]->GetRSEdge( cnt1)->cuttingProbes[cnt2]->GetProbeCenter().GetZ();
                        counter++;
                    }
                    else
                    {
                        singTexData[counter] = 0.0f;
                        counter++;
                        singTexData[counter] = 0.0f;
                        counter++;
                        singTexData[counter] = 0.0f;
                        counter++;
                    }
                }
            }
        }
        // texture generation
        glBindTexture( GL_TEXTURE_2D, singularityTexture[cntRS]);
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, this->singTexWidth[cntRS], this->singTexHeight[cntRS], 0, GL_RGB, GL_FLOAT, this->singTexData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture( GL_TEXTURE_2D, 0);
    }
/*
    std::cout << "Create texture: " <<
        ( double( clock() - t) / double( CLOCKS_PER_SEC) ) << std::endl;
*/
}


/*
 * Creates the texture for singularity handling.
 */
void ProteinRendererBDP::CreateSingularityTexturesSimple()
{
/*
    time_t t = clock();
*/
    unsigned int cnt1, cnt2, cntRS;
    
    // delete old singularity textures
    for( cnt1 = 0; cnt1 < this->singularityTexture.size(); ++cnt1 )
    {
        glDeleteTextures( 1, &singularityTexture[cnt1]);
    }
    // check if the singularity texture has the right size
    if( this->simpleRS.size() != this->singularityTexture.size() )
    {
        // store old singularity texture size
        unsigned int singTexSizeOld = (unsigned int)this->singularityTexture.size();
        // resize singularity texture to fit the number of reduced surfaces
        this->singularityTexture.resize( this->simpleRS.size());
        // generate a new texture for each new singularity texture
        for( cnt1 = singTexSizeOld; cnt1 < singularityTexture.size(); ++cnt1 )
        {
            glGenTextures( 1, &singularityTexture[cnt1]);
        }
    }
    // resize singularity texture dimension arrays
    this->singTexWidth.resize( this->simpleRS.size());
    this->singTexHeight.resize( this->simpleRS.size());
    
    // get maximum texture size
    GLint texSize;
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &texSize);
        
    // TODO: compute proper maximum number of cutting probes
    unsigned int numProbes = 16;
    
    for( cntRS = 0; cntRS < this->simpleRS.size(); ++cntRS )
    {
        // set width and height of texture
        if( (unsigned int)texSize < this->simpleRS[cntRS]->GetCutRSEdgesCount() )
        {
            this->singTexHeight[cntRS] = texSize;
            this->singTexWidth[cntRS] = numProbes * (int)ceil( double(
                this->simpleRS[cntRS]->GetCutRSEdgesCount()) / (double)texSize);
        }
        else
        {
            this->singTexHeight[cntRS] = this->simpleRS[cntRS]->GetCutRSEdgesCount();
            this->singTexWidth[cntRS] = numProbes;
        }
        // generate float-array for texture with the appropriate dimension
        if( this->singTexData )
            delete[] this->singTexData;
        this->singTexData = new float[this->singTexWidth[cntRS]*this->singTexHeight[cntRS]*3];
        // write probes to singularity texture
        unsigned int coordX = 0;
        unsigned int coordY = 0;
        unsigned int counter = 0;
        for( cnt1 = 0; cnt1 < this->simpleRS[cntRS]->GetRSEdgeCount(); ++cnt1 )
        {
            if( this->simpleRS[cntRS]->GetRSEdge( cnt1)->cuttingProbes.empty() )
            {
                this->simpleRS[cntRS]->GetRSEdge( cnt1)->SetTexCoord( 0, 0);
            }
            else
            {
                // set texture coordinates
                this->simpleRS[cntRS]->GetRSEdge( cnt1)->SetTexCoord( coordX, coordY);
                // compute texture coordinates for next entry
                coordY++;
                if( coordY == this->singTexHeight[cntRS] )
                {
                    coordY = 0;
                    coordX = coordX + numProbes;
                }
                // write probes to texture
                for( cnt2 = 0; cnt2 < numProbes; ++cnt2 )
                {
                    if( cnt2 < this->simpleRS[cntRS]->GetRSEdge( cnt1)->cuttingProbes.size() )
                    {
                        singTexData[counter] =
                                this->simpleRS[cntRS]->GetRSEdge( cnt1)->cuttingProbes[cnt2]->GetProbeCenter().GetX();
                        counter++;
                        singTexData[counter] =
                                this->simpleRS[cntRS]->GetRSEdge( cnt1)->cuttingProbes[cnt2]->GetProbeCenter().GetY();
                        counter++;
                        singTexData[counter] =
                                this->simpleRS[cntRS]->GetRSEdge( cnt1)->cuttingProbes[cnt2]->GetProbeCenter().GetZ();
                        counter++;
                    }
                    else
                    {
                        singTexData[counter] = 0.0f;
                        counter++;
                        singTexData[counter] = 0.0f;
                        counter++;
                        singTexData[counter] = 0.0f;
                        counter++;
                    }
                }
            }
        }
        // texture generation
        glBindTexture( GL_TEXTURE_2D, singularityTexture[cntRS]);
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, this->singTexWidth[cntRS], this->singTexHeight[cntRS], 0, GL_RGB, GL_FLOAT, this->singTexData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture( GL_TEXTURE_2D, 0);
    }
/*
    std::cout << "Create texture: " <<
        ( double( clock() - t) / double( CLOCKS_PER_SEC) ) << std::endl;
*/
}


/*
 * Creates the texture for singularity handling.
 */
void ProteinRendererBDP::CreateSingularityTexture( unsigned int idxRS)
{
    // do nothing if the index is out of bounds
    if( idxRS > this->reducedSurface.size() )
        return;
    
    // check if all arrays have the appropriate size
    if( this->singularityTexture.size() != this->reducedSurface.size() ||
        this->singTexWidth.size() != this->reducedSurface.size() ||
        this->singTexHeight.size() != this->reducedSurface.size() )
    {
        // create all singularity textures
        CreateSingularityTextures();
        return;
    }
    
    unsigned int cnt1, cnt2;
    
    // delete old singularity texture
    glDeleteTextures( 1, &singularityTexture[idxRS]);
    
    // get maximum texture size
    GLint texSize;
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &texSize);
        
    // TODO: compute proper maximum number of cutting probes
    unsigned int numProbes = 16;

    // set width and height of texture
    if( (unsigned int)texSize < this->reducedSurface[idxRS]->GetCutRSEdgesCount() )
    {
        this->singTexHeight[idxRS] = texSize;
        this->singTexWidth[idxRS] = numProbes * (int)ceil( double(
            this->reducedSurface[idxRS]->GetCutRSEdgesCount()) / (double)texSize);
    }
    else
    {
        this->singTexHeight[idxRS] = this->reducedSurface[idxRS]->GetCutRSEdgesCount();
        this->singTexWidth[idxRS] = numProbes;
    }
    // generate float-array for texture with the appropriate dimension
    if( this->singTexData )
        delete[] this->singTexData;
    this->singTexData = new float[this->singTexWidth[idxRS]*this->singTexHeight[idxRS]*3];
    // write probes to singularity texture
    unsigned int coordX = 0;
    unsigned int coordY = 0;
    unsigned int counter = 0;
    for( cnt1 = 0; cnt1 < this->reducedSurface[idxRS]->GetRSEdgeCount(); ++cnt1 )
    {
        if( this->reducedSurface[idxRS]->GetRSEdge( cnt1)->cuttingProbes.empty() )
        {
            this->reducedSurface[idxRS]->GetRSEdge( cnt1)->SetTexCoord( 0, 0);
        }
        else
        {
            // set texture coordinates
            this->reducedSurface[idxRS]->GetRSEdge( cnt1)->SetTexCoord( coordX, coordY);
            // compute texture coordinates for next entry
            coordY++;
            if( coordY == this->singTexHeight[idxRS] )
            {
                coordY = 0;
                coordX = coordX + numProbes;
            }
            // write probes to texture
            for( cnt2 = 0; cnt2 < numProbes; ++cnt2 )
            {
                if( cnt2 < this->reducedSurface[idxRS]->GetRSEdge( cnt1)->cuttingProbes.size() )
                {
                    singTexData[counter] =
                            this->reducedSurface[idxRS]->GetRSEdge( cnt1)->cuttingProbes[cnt2]->GetProbeCenter().GetX();
                    counter++;
                    singTexData[counter] =
                            this->reducedSurface[idxRS]->GetRSEdge( cnt1)->cuttingProbes[cnt2]->GetProbeCenter().GetY();
                    counter++;
                    singTexData[counter] =
                            this->reducedSurface[idxRS]->GetRSEdge( cnt1)->cuttingProbes[cnt2]->GetProbeCenter().GetZ();
                    counter++;
                }
                else
                {
                    singTexData[counter] = 0.0f;
                    counter++;
                    singTexData[counter] = 0.0f;
                    counter++;
                    singTexData[counter] = 0.0f;
                    counter++;
                }
            }
        }
    }
    // texture generation
    glBindTexture( GL_TEXTURE_2D, singularityTexture[idxRS]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F_ARB, this->singTexWidth[idxRS], this->singTexHeight[idxRS], 0, GL_RGB, GL_FLOAT, this->singTexData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture( GL_TEXTURE_2D, 0);
    
}


/*
 * Render all atoms
 */
void ProteinRendererBDP::RenderAtomsGPU( const CallProteinData *protein, const float scale)
{
    unsigned int cnt, cntRS, max1, max2;

    // set viewport
    float viewportStuff[4] =
    {
        cameraInfo->TileRect().Left(),
        cameraInfo->TileRect().Bottom(),
        cameraInfo->TileRect().Width(),
        cameraInfo->TileRect().Height()
    };
    if (viewportStuff[2] < 1.0f) viewportStuff[2] = 1.0f;
    if (viewportStuff[3] < 1.0f) viewportStuff[3] = 1.0f;
    viewportStuff[2] = 2.0f / viewportStuff[2];
    viewportStuff[3] = 2.0f / viewportStuff[3];

    // enable sphere shader
    this->sphereShader.Enable();
    // set shader variables
    glUniform4fvARB(this->sphereShader.ParameterLocation("viewAttr"), 1, viewportStuff);
    glUniform3fvARB(this->sphereShader.ParameterLocation("camIn"), 1, cameraInfo->Front().PeekComponents());
    glUniform3fvARB(this->sphereShader.ParameterLocation("camRight"), 1, cameraInfo->Right().PeekComponents());
    glUniform3fvARB(this->sphereShader.ParameterLocation("camUp"), 1, cameraInfo->Up().PeekComponents());

    glBegin( GL_POINTS);

    glColor3f( 1.0f, 0.0f, 0.0f);
    if( this->currentRendermode == GPU_SIMPLIFIED )
        max1 = (unsigned int)this->simpleRS.size();
    else
        max1 = (unsigned int)this->reducedSurface.size();
    for( cntRS = 0; cntRS < max1; ++cntRS )
    {
        if( this->currentRendermode == GPU_SIMPLIFIED )
            max2 = this->simpleRS[cntRS]->GetRSVertexCount();
        else
            max2 = this->reducedSurface[cntRS]->GetRSVertexCount();
        // loop over all protein atoms
        for( cnt = 0; cnt < max2; ++cnt )
        {
            if( this->currentRendermode == GPU_SIMPLIFIED )
            {
                //glColor3ubv( protein->AtomTypes()[protein->ProteinAtomData()[this->simpleRS[cntRS]->GetRSVertex( cnt)->GetIndex()].TypeIndex()].Colour());
                glColor3f( 1.0f, 0.0f, 0.0f);
                glVertex4f(
                    this->simpleRS[cntRS]->GetRSVertex( cnt)->GetPosition().GetX(),
                    this->simpleRS[cntRS]->GetRSVertex( cnt)->GetPosition().GetY(),
                    this->simpleRS[cntRS]->GetRSVertex( cnt)->GetPosition().GetZ(),
                    this->simpleRS[cntRS]->GetRSVertex( cnt)->GetRadius()*scale );
            }
            else
            {
                if( this->reducedSurface[cntRS]->GetRSVertex( cnt)->IsBuried() )
                    continue;
                //glColor3ubv( protein->AtomTypes()[protein->ProteinAtomData()[this->reducedSurface[cntRS]->GetRSVertex( cnt)->GetIndex()].TypeIndex()].Colour());
                glColor3f( 1.0f, 0.0f, 0.0f);
                glVertex4f(
                    this->reducedSurface[cntRS]->GetRSVertex( cnt)->GetPosition().GetX(),
                    this->reducedSurface[cntRS]->GetRSVertex( cnt)->GetPosition().GetY(),
                    this->reducedSurface[cntRS]->GetRSVertex( cnt)->GetPosition().GetZ(),
                    this->reducedSurface[cntRS]->GetRSVertex( cnt)->GetRadius()*scale );
            }
        }
    }

    glEnd(); // GL_POINTS

    // disable sphere shader
    this->sphereShader.Disable();
}


/*
 * Renders the probe at postion 'm'
 */
void ProteinRendererBDP::RenderProbe( 
    const vislib::math::Vector<float, 3> m)
{
    GLUquadricObj *sphere = gluNewQuadric();
    gluQuadricNormals( sphere, GL_SMOOTH);

    glEnable( GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE);

    glPushMatrix();
    glTranslatef( m.GetX(), m.GetY(), m.GetZ());
    glColor4f( 1.0f, 1.0f, 1.0f, 0.6f);
    gluSphere( sphere, probeRadius, 16, 8);
    glPopMatrix();
    
    glDisable( GL_BLEND);

}


/*
 * Renders the probe at postion 'm'
 */
void ProteinRendererBDP::RenderProbeGPU( 
    const vislib::math::Vector<float, 3> m)
{
    // set viewport
    float viewportStuff[4] =
    {
        cameraInfo->TileRect().Left(),
        cameraInfo->TileRect().Bottom(),
        cameraInfo->TileRect().Width(),
        cameraInfo->TileRect().Height()
    };
    if (viewportStuff[2] < 1.0f) viewportStuff[2] = 1.0f;
    if (viewportStuff[3] < 1.0f) viewportStuff[3] = 1.0f;
    viewportStuff[2] = 2.0f / viewportStuff[2];
    viewportStuff[3] = 2.0f / viewportStuff[3];

    // enable sphere shader
    this->sphereShader.Enable();
    // set shader variables
    glUniform4fvARB(this->sphereShader.ParameterLocation("viewAttr"), 1, viewportStuff);
    glUniform3fvARB(this->sphereShader.ParameterLocation("camIn"), 1, cameraInfo->Front().PeekComponents());
    glUniform3fvARB(this->sphereShader.ParameterLocation("camRight"), 1, cameraInfo->Right().PeekComponents());
    glUniform3fvARB(this->sphereShader.ParameterLocation("camUp"), 1, cameraInfo->Up().PeekComponents());

    glBegin( GL_POINTS);
        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex4f( m.GetX(), m.GetY(), m.GetZ(), probeRadius );
    glEnd();

    // disable sphere shader
    this->sphereShader.Disable();
}


/*
 * ProteinRendererBDP::deinitialise
 */
void ProteinRendererBDP::deinitialise(void)
{
    if( colorFBO )
    {
        glDeleteFramebuffersEXT( 1, &colorFBO);
        glDeleteFramebuffersEXT( 1, &blendFBO);
        glDeleteFramebuffersEXT( 1, &horizontalFilterFBO);
        glDeleteFramebuffersEXT( 1, &verticalFilterFBO);
        glDeleteTextures( 1, &texture0);
        glDeleteTextures( 1, &depthTex0);
        glDeleteTextures( 1, &texture1);
        glDeleteTextures( 1, &depthTex1);
        glDeleteTextures( 1, &hFilter);
        glDeleteTextures( 1, &vFilter);
    }
    // delete singularity texture
    for( unsigned int i = 0; i < singularityTexture.size(); ++i )
        glDeleteTextures( 1, &singularityTexture[i]);
    // release shaders
    this->cylinderShader.Release();
    this->sphereShader.Release();
    this->sphereClipInteriorShader.Release();
    this->sphericalTriangleShader.Release();
    this->torusShader.Release();
    this->lightShader.Release();
    this->hfilterShader.Release();
    this->vfilterShader.Release();
    this->silhouetteShader.Release();
    this->transparencyShader.Release();
}


/*
 * Fill amino acid color table
 */
void ProteinRendererBDP::FillAminoAcidColorTable()
{
    this->aminoAcidColorTable.Clear();
    this->aminoAcidColorTable.SetCount( 25);
    this->aminoAcidColorTable[0].Set( 0.5f, 0.5f, 0.5f);
    this->aminoAcidColorTable[1].Set( 1.0f, 0.0f, 0.0f);
    this->aminoAcidColorTable[2].Set( 1.0f, 1.0f, 0.0f);
    this->aminoAcidColorTable[3].Set( 0.0f, 1.0f, 0.0f);
    this->aminoAcidColorTable[4].Set( 0.0f, 1.0f, 1.0f);
    this->aminoAcidColorTable[5].Set( 0.0f, 0.0f, 1.0f);
    this->aminoAcidColorTable[6].Set( 1.0f, 0.0f, 1.0f);
    this->aminoAcidColorTable[7].Set( 0.5f, 0.0f, 0.0f);
    this->aminoAcidColorTable[8].Set( 0.5f, 0.5f, 0.0f);
    this->aminoAcidColorTable[9].Set( 0.0f, 0.5f, 0.0f);
    this->aminoAcidColorTable[10].Set( 0.0f, 0.5f, 0.5f);
    this->aminoAcidColorTable[11].Set( 0.0f, 0.0f, 0.5f);
    this->aminoAcidColorTable[12].Set( 0.5f, 0.0f, 0.5f);
    this->aminoAcidColorTable[13].Set( 1.0f, 0.5f, 0.0f);
    this->aminoAcidColorTable[14].Set( 0.0f, 0.5f, 1.0f);
    this->aminoAcidColorTable[15].Set( 1.0f, 0.5f, 1.0f);
    this->aminoAcidColorTable[16].Set( 0.5f, 0.25f, 0.0f);
    this->aminoAcidColorTable[17].Set( 1.0f, 1.0f, 0.5f);
    this->aminoAcidColorTable[18].Set( 0.5f, 1.0f, 0.5f);
    this->aminoAcidColorTable[19].Set( 0.75f, 1.0f, 0.0f);
    this->aminoAcidColorTable[20].Set( 0.5f, 0.0f, 0.75f);
    this->aminoAcidColorTable[21].Set( 1.0f, 0.5f, 0.5f);
    this->aminoAcidColorTable[22].Set( 0.75f, 1.0f, 0.75f);
    this->aminoAcidColorTable[23].Set( 0.75f, 0.75f, 0.5f);
    this->aminoAcidColorTable[24].Set( 1.0f, 0.75f, 0.5f);
}


/*
 * protein::ProteinRendererBDP::makeRainbowColorTable
 * Creates a rainbow color table with 'num' entries.
 */
void ProteinRendererBDP::MakeRainbowColorTable( unsigned int num)
{
    unsigned int n = (num/4);
    // the color table should have a minimum size of 16
    if( n < 4 )
        n = 4;
    this->rainbowColors.clear();
    float f = 1.0f/float(n);
    vislib::math::Vector<float,3> color;
    color.Set( 1.0f, 0.0f, 0.0f);
    for( unsigned int i = 0; i < n; i++)
    {
        color.SetY( vislib::math::Min( color.GetY() + f, 1.0f));
        rainbowColors.push_back( color);
    }
    for( unsigned int i = 0; i < n; i++)
    {
        color.SetX( vislib::math::Max( color.GetX() - f, 0.0f));
        rainbowColors.push_back( color);
    }
    for( unsigned int i = 0; i < n; i++)
    {
        color.SetZ( vislib::math::Min( color.GetZ() + f, 1.0f));
        rainbowColors.push_back( color);
    }
    for( unsigned int i = 0; i < n; i++)
    {
        color.SetY( vislib::math::Max( color.GetY() - f, 0.0f));
        rainbowColors.push_back( color);
    }
}


/*
 * returns the color of the atom 'idx' for the current coloring mode
 */
vislib::math::Vector<float, 3> ProteinRendererBDP::GetProteinAtomColor( unsigned int idx)
{
    if( idx < this->atomColor.size() )
        return this->atomColor[idx];
    else
        return vislib::math::Vector<float, 3>( 0.5f, 0.5f, 0.5f);
}


/*
 * Make color table for all atoms
 */
void ProteinRendererBDP::MakeColorTable( const CallProteinData *prot, bool forceRecompute)
{
    unsigned int i;
    unsigned int currentChain, currentAminoAcid, currentAtom, currentSecStruct;
    unsigned int cntCha, cntRes, cntAto;
    CallProteinData::Chain chain;
    vislib::math::Vector<float, 3> color;
    // if recomputation is forced: clear current color table
    if( forceRecompute )
        this->atomColor.clear();
    // only compute color table if necessary
    if( this->atomColor.empty() )
    {
        this->atomColor.reserve( prot->ProteinAtomCount());
        if( this->currentColoringMode == ELEMENT )
        {
            // resize atom color table
            this->atomColor.resize( prot->ProteinAtomCount());
            for( i = 0; i < prot->ProteinAtomCount(); i++ )
            {
                this->atomColor[i].SetX( float(prot->AtomTypes()[prot->ProteinAtomData()[i].TypeIndex()].Colour()[0]) / 255.0f);
                this->atomColor[i].SetY( float(prot->AtomTypes()[prot->ProteinAtomData()[i].TypeIndex()].Colour()[1]) / 255.0f);
                this->atomColor[i].SetZ( float(prot->AtomTypes()[prot->ProteinAtomData()[i].TypeIndex()].Colour()[2]) / 255.0f);
            }
        } // ===== END coloring mode ELEMENT =====
        else if( this->currentColoringMode == AMINOACID )
        {
            // loop over all chains
            for( currentChain = 0; currentChain < prot->ProteinChainCount(); currentChain++ )
            {
                chain = prot->ProteinChain( currentChain);
                // loop over all amino acids in the current chain
                for( currentAminoAcid = 0; currentAminoAcid < chain.AminoAcidCount(); currentAminoAcid++ )
                {
                    // loop over all connections of the current amino acid
                    for( currentAtom = 0; 
                         currentAtom < chain.AminoAcid()[currentAminoAcid].AtomCount();
                         currentAtom++ )
                    {
                        i = chain.AminoAcid()[currentAminoAcid].NameIndex()+1;
                        i = i % (unsigned int)(this->aminoAcidColorTable.Count());
                        this->atomColor.push_back( this->aminoAcidColorTable[i]);
                    }
                }
            }
        } // ===== END coloring mode AMINOACID =====
        else if( this->currentColoringMode == STRUCTURE )
        {
            // loop over all chains
            for( currentChain = 0; currentChain < prot->ProteinChainCount(); currentChain++ )
            {
                chain = prot->ProteinChain( currentChain);
                // loop over all secondary structure elements in this chain
                for( currentSecStruct = 0; 
                     currentSecStruct < chain.SecondaryStructureCount();
                     currentSecStruct++ )
                {
                    i = chain.SecondaryStructure()[currentSecStruct].AtomCount();
                    // loop over all atoms in this secondary structure element
                    for( currentAtom = 0; currentAtom < i; currentAtom++ )
                    {
                        if( chain.SecondaryStructure()[currentSecStruct].Type() ==
                            CallProteinData::SecStructure::TYPE_HELIX )
                        {
                            this->atomColor.push_back( vislib::math::Vector<float, 3>( 1.0f, 0.0f, 0.0f));
                        }
                        else if( chain.SecondaryStructure()[currentSecStruct].Type() ==
                            CallProteinData::SecStructure::TYPE_SHEET )
                        {
                            this->atomColor.push_back( vislib::math::Vector<float, 3>( 0.0f, 0.0f, 1.0f));
                        }
                        else if( chain.SecondaryStructure()[currentSecStruct].Type() ==
                            CallProteinData::SecStructure::TYPE_TURN )
                        {
                            this->atomColor.push_back( vislib::math::Vector<float, 3>( 1.0f, 1.0f, 0.0f));
                        }
                        else
                        {
                            this->atomColor.push_back( vislib::math::Vector<float, 3>( 0.9f, 0.9f, 0.9f));
                        }
                    }
                }
            }
            // add missing atom colors
            if ( prot->ProteinAtomCount() > this->atomColor.size() )
            {
                currentAtom = (unsigned int)this->atomColor.size();
                for ( ; currentAtom < prot->ProteinAtomCount(); ++currentAtom )
                {
                    this->atomColor.push_back( vislib::math::Vector<float, 3>( 0.3f, 0.9f, 0.3f));
                }
            }
        } // ===== END coloring mode STRUCTURE =====
        else if( this->currentColoringMode == CHAIN_ID )
        {
            // loop over all chains
            for( currentChain = 0; currentChain < prot->ProteinChainCount(); currentChain++ )
            {
                chain = prot->ProteinChain( currentChain);
                // loop over all amino acids in the current chain
                for( currentAminoAcid = 0; currentAminoAcid < chain.AminoAcidCount(); currentAminoAcid++ )
                {
                    // loop over all connections of the current amino acid
                    for( currentAtom = 0; 
                         currentAtom < chain.AminoAcid()[currentAminoAcid].AtomCount();
                         currentAtom++ )
                    {
                        i = (currentChain + 1) % (unsigned int)(this->aminoAcidColorTable.Count());
                        this->atomColor.push_back( this->aminoAcidColorTable[i]);
                    }
                }
            }
        } // ===== END coloring mode CHAIN_ID =====
        else if( this->currentColoringMode == VALUE )
        {
            vislib::math::Vector<float, 3> colMax = this->maxValueColor;
            vislib::math::Vector<float, 3> colMid = this->meanValueColor;
            vislib::math::Vector<float, 3> colMin = this->minValueColor;
            
            float min( prot->MinimumTemperatureFactor() );
            float max( prot->MaximumTemperatureFactor() );
            float mid( ( max - min)/2.0f + min );
            float val;
            
            for ( i = 0; i < prot->ProteinAtomCount(); i++ )
            {
                if( min == max )
                {
                    this->atomColor.push_back( colMid);
                    continue;
                }
                
                val = prot->ProteinAtomData()[i].TempFactor();
                // below middle value --> blend between min and mid color
                if( val < mid )
                {
                    color = colMin + ( ( colMid - colMin ) / ( mid - min) ) * ( val - min );
                    this->atomColor.push_back( color);
                }
                // above middle value --> blend between max and mid color
                else if( val > mid )
                {
                    color = colMid + ( ( colMax - colMid ) / ( max - mid) ) * ( val - mid );
                    this->atomColor.push_back( color);
                }
                // middle value --> assign mid color
                else
                {
                    this->atomColor.push_back( colMid);
                }
            }
        } // ===== END coloring mode VALUE =====
        else if( this->currentColoringMode == RAINBOW )
        {
            for( cntCha = 0; cntCha < prot->ProteinChainCount(); ++cntCha )
            {
                for( cntRes = 0; cntRes < prot->ProteinChain( cntCha).AminoAcidCount(); ++cntRes )
                {
                    i = int( ( float( cntRes) / float( prot->ProteinChain( cntCha).AminoAcidCount() ) ) * float( rainbowColors.size() ) ) % rainbowColors.size();
                    color = this->rainbowColors[i];
                    for( cntAto = 0;
                         cntAto < prot->ProteinChain( cntCha).AminoAcid()[cntRes].AtomCount();
                         ++cntAto )
                    {
                        this->atomColor.push_back( color);
                    }
                }
            }
        } // ===== END coloring mode RAINBOW =====
        else if ( this->currentColoringMode == CHARGE )
        {
            vislib::math::Vector<float, 3> colMax = this->maxValueColor;
            vislib::math::Vector<float, 3> colMid = this->meanValueColor;
            vislib::math::Vector<float, 3> colMin = this->minValueColor;
            
            float min( prot->MinimumCharge() );
            float max( prot->MaximumCharge() );
            float mid( ( max - min)/2.0f + min );
            float charge;
            
            for ( i = 0; i < prot->ProteinAtomCount(); i++ )
            {
                if( min == max )
                {
                    this->atomColor.push_back( colMid);
                    continue;
                }
                
                charge = prot->ProteinAtomData()[i].Charge();
                // below middle value --> blend between min and mid color
                if( charge < mid )
                {
                    color = colMin + ( ( colMid - colMin ) / ( mid - min) ) * ( charge - min );
                    this->atomColor.push_back( color);
                }
                // above middle value --> blend between max and mid color
                else if( charge > mid )
                {
                    color = colMid + ( ( colMax - colMid ) / ( max - mid) ) * ( charge - mid );
                    this->atomColor.push_back( color);
                }
                // middle value --> assign mid color
                else
                {
                    this->atomColor.push_back( colMid);
                }
            }
        } // ===== END coloring mode CHARGE =====
        else // default case (wrong parameter):
        {
            // resize atom color table
            this->atomColor.resize( prot->ProteinAtomCount());
            for( i = 0; i < prot->ProteinAtomCount(); i++ )
            {
                this->atomColor[i].SetX( 1.0f);
                this->atomColor[i].SetY( 1.0f);
                this->atomColor[i].SetZ( 1.0f);
            }
        } // ===== END uniform color =====
    }
}
