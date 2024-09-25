mmCheckVersion("852622f72f52449f")
mmCreateView("GraphEntry_1","View3DGL","::View3DGL_1")

mmCreateModule("SphereRenderer","::Group_1::SphereRenderer_1")
mmCreateModule("DistantLight","::Group_2::DistantLight_1")
mmCreateModule("MoleculeBallifier","::Group_1::MoleculeBallifier_1")
mmCreateModule("PDBLoader","::Group_1::PDBLoader_1")
mmCreateModule("SimpleRenderTarget","::Group_1::SimpleRenderTarget_1")
mmCreateModule("LocalLighting","::Group_2::LocalLighting_1")
mmCreateModule("DrawToScreen","::DrawToScreen_1")
mmCreateModule("OpticalFlow","::OpticalFlow_1")
mmCreateModule("AmbientLight","::Group_2::AmbientLight_1")
mmCreateModule("MotionBlur","::MotionBlur_1")

mmCreateCall("CallRender3DGL","::View3DGL_1::rendering","::DrawToScreen_1::rendering")
mmCreateCall("MultiParticleDataCall","::Group_1::SphereRenderer_1::getdata","::Group_1::MoleculeBallifier_1::outData")
mmCreateCall("CallLight","::Group_2::DistantLight_1::getLightSlot","::Group_2::AmbientLight_1::deployLightSlot")
mmCreateCall("MolecularDataCall","::Group_1::MoleculeBallifier_1::inData","::Group_1::PDBLoader_1::dataout")
mmCreateCall("CallRender3DGL","::Group_1::SimpleRenderTarget_1::chainRendering","::Group_1::SphereRenderer_1::rendering")
mmCreateCall("CallTexture2D","::Group_2::LocalLighting_1::AlbedoTexture","::Group_1::SimpleRenderTarget_1::Color")
mmCreateCall("CallTexture2D","::Group_2::LocalLighting_1::NormalTexture","::Group_1::SimpleRenderTarget_1::Normals")
mmCreateCall("CallTexture2D","::Group_2::LocalLighting_1::DepthTexture","::Group_1::SimpleRenderTarget_1::Depth")
mmCreateCall("CallLight","::Group_2::LocalLighting_1::lights","::Group_2::DistantLight_1::deployLightSlot")
mmCreateCall("CallCamera","::Group_2::LocalLighting_1::Camera","::Group_1::SimpleRenderTarget_1::Camera")
mmCreateCall("CallRender3DGL","::DrawToScreen_1::chainRendering","::Group_1::SimpleRenderTarget_1::rendering")
mmCreateCall("CallTexture2D","::DrawToScreen_1::InputTexture","::OpticalFlow_1::FlowFieldTexture")
mmCreateCall("CallTexture2D","::DrawToScreen_1::DepthTexture","::Group_1::SimpleRenderTarget_1::Normals")
mmCreateCall("CallTexture2D","::OpticalFlow_1::InputTexture","::Group_2::LocalLighting_1::OutputTexture")
mmCreateCall("CallTexture2D","::MotionBlur_1::ColorTexture","::Group_1::SimpleRenderTarget_1::Color")
mmCreateCall("CallTexture2D","::MotionBlur_1::DepthTexture","::Group_1::SimpleRenderTarget_1::Depth")
mmCreateCall("CallTexture2D","::MotionBlur_1::FlowTexture","::OpticalFlow_1::FlowFieldTexture")

mmSetParamValue("::View3DGL_1::camstore::settings",[=[]=])
mmSetParamValue("::View3DGL_1::multicam::overrideSettings",[=[false]=])
mmSetParamValue("::View3DGL_1::multicam::autoSaveSettings",[=[false]=])
mmSetParamValue("::View3DGL_1::multicam::autoLoadSettings",[=[true]=])
mmSetParamValue("::View3DGL_1::resetViewOnBBoxChange",[=[false]=])
mmSetParamValue("::View3DGL_1::showLookAt",[=[false]=])
mmSetParamValue("::View3DGL_1::view::showViewCube",[=[false]=])
mmSetParamValue("::View3DGL_1::anim::play",[=[true]=])
mmSetParamValue("::View3DGL_1::anim::speed",[=[57.000000]=])
mmSetParamValue("::View3DGL_1::anim::time",[=[68.523438]=])
mmSetParamValue("::View3DGL_1::backCol",[=[#000020]=])
mmSetParamValue("::View3DGL_1::viewKey::MoveStep",[=[0.500000]=])
mmSetParamValue("::View3DGL_1::viewKey::RunFactor",[=[2.000000]=])
mmSetParamValue("::View3DGL_1::viewKey::AngleStep",[=[90.000000]=])
mmSetParamValue("::View3DGL_1::viewKey::FixToWorldUp",[=[true]=])
mmSetParamValue("::View3DGL_1::viewKey::MouseSensitivity",[=[3.000000]=])
mmSetParamValue("::View3DGL_1::viewKey::RotPoint",[=[Look-At]=])
mmSetParamValue("::View3DGL_1::view::cubeOrientation",[=[0.0736922622;0.925341904;0.240101889;-0.284010977]=])
mmSetParamValue("::View3DGL_1::view::defaultView",[=[FACE - Front]=])
mmSetParamValue("::View3DGL_1::view::defaultOrientation",[=[Top]=])
mmSetParamValue("::View3DGL_1::cam::position",[=[-62.452774;146.498596;-74.5787811]=])
mmSetParamValue("::View3DGL_1::cam::orientation",[=[0.0736922622;0.925341904;0.240101889;-0.284010977]=])
mmSetParamValue("::View3DGL_1::cam::projectiontype",[=[Perspective]=])
mmSetParamValue("::View3DGL_1::cam::nearplane",[=[162.567963]=])
mmSetParamValue("::View3DGL_1::cam::farplane",[=[290.553345]=])
mmSetParamValue("::View3DGL_1::cam::halfaperturedegrees",[=[28.647890]=])
mmSetParamValue("::Group_1::SphereRenderer_1::shaderMode",[=[Deferred]=])
mmSetParamValue("::Group_1::SphereRenderer_1::scaling",[=[1.000000]=])
mmSetParamValue("::Group_1::SphereRenderer_1::forceTime",[=[false]=])
mmSetParamValue("::Group_1::SphereRenderer_1::useLocalBbox",[=[false]=])
mmSetParamValue("::Group_1::SphereRenderer_1::flag storage::selectedColor",[=[#ff0000]=])
mmSetParamValue("::Group_1::SphereRenderer_1::flag storage::softSelectedColor",[=[#ff8080]=])
mmSetParamValue("::Group_1::SphereRenderer_1::splat::alphaScaling",[=[5.000000]=])
mmSetParamValue("::Group_1::SphereRenderer_1::splat::attenuateSubpixel",[=[false]=])
mmSetParamValue("::Group_1::SphereRenderer_1::ssbo::staticData",[=[false]=])
mmSetParamValue("::Group_1::SphereRenderer_1::ambient occlusion::enableLighting",[=[false]=])
mmSetParamValue("::Group_1::SphereRenderer_1::ambient occlusion::useGsProxies",[=[false]=])
mmSetParamValue("::Group_1::SphereRenderer_1::ambient occlusion::volumeSize",[=[128]=])
mmSetParamValue("::Group_1::SphereRenderer_1::ambient occlusion::apex",[=[50.000000]=])
mmSetParamValue("::Group_1::SphereRenderer_1::ambient occlusion::offset",[=[0.010000]=])
mmSetParamValue("::Group_1::SphereRenderer_1::ambient occlusion::strength",[=[1.000000]=])
mmSetParamValue("::Group_1::SphereRenderer_1::ambient occlusion::coneLength",[=[0.800000]=])
mmSetParamValue("::Group_1::SphereRenderer_1::ambient occlusion::highPrecisionTexture",[=[false]=])
mmSetParamValue("::Group_1::SphereRenderer_1::outline::width",[=[2.000000]=])
mmSetParamValue("::Group_1::SphereRenderer_1::renderMode",[=[Ambient_Occlusion]=])
mmSetParamValue("::Group_2::DistantLight_1::Intensity",[=[1.000000]=])
mmSetParamValue("::Group_2::DistantLight_1::Color",[=[#cccccc]=])
mmSetParamValue("::Group_2::DistantLight_1::Direction",[=[-0.589376509;0.014245877;-0.726242602]=])
mmSetParamValue("::Group_2::DistantLight_1::AngularDiameter",[=[0.000000]=])
mmSetParamValue("::Group_2::DistantLight_1::EyeDirection",[=[false]=])
mmSetParamValue("::Group_1::MoleculeBallifier_1::colorTableFilename",[=[colors.txt]=])
mmSetParamValue("::Group_1::MoleculeBallifier_1::coloringMode0",[=[Element]=])
mmSetParamValue("::Group_1::MoleculeBallifier_1::coloringMode1",[=[Element]=])
mmSetParamValue("::Group_1::MoleculeBallifier_1::colorWeight",[=[0.500000]=])
mmSetParamValue("::Group_1::MoleculeBallifier_1::minGradColor",[=[#146496]=])
mmSetParamValue("::Group_1::MoleculeBallifier_1::midGradColor",[=[#f0f0f0]=])
mmSetParamValue("::Group_1::MoleculeBallifier_1::maxGradColor",[=[#ae3b32]=])
mmSetParamValue("::Group_1::MoleculeBallifier_1::specialColor",[=[#228b22]=])
mmSetParamValue("::Group_1::PDBLoader_1::pdbFilename",[=[/home/masta76/Schreibtisch/Thesis/megamol-visual-effects/thesis_projects/animation_ballifier/2veo03.pdb]=])
mmSetParamValue("::Group_1::PDBLoader_1::xtcFilename",[=[/home/masta76/Schreibtisch/Thesis/megamol-visual-effects/thesis_projects/animation_ballifier/2veo03.xtc]=])
mmSetParamValue("::Group_1::PDBLoader_1::capFilename",[=[]=])
mmSetParamValue("::Group_1::PDBLoader_1::maxFrames",[=[500]=])
mmSetParamValue("::Group_1::PDBLoader_1::strideFlag",[=[true]=])
mmSetParamValue("::Group_1::PDBLoader_1::solventResidues",[=[]=])
mmSetParamValue("::Group_1::PDBLoader_1::calcBBoxPerFrame",[=[false]=])
mmSetParamValue("::Group_1::PDBLoader_1::calculateBonds",[=[true]=])
mmSetParamValue("::Group_1::PDBLoader_1::recomputeSTRIDEeachFrame",[=[false]=])
mmSetParamValue("::Group_1::SimpleRenderTarget_1::color Out Format",[=[GL_RGBA8_SNORM]=])
mmSetParamValue("::Group_1::SimpleRenderTarget_1::normals Out Format",[=[GL_RGB16F]=])
mmSetParamValue("::Group_2::LocalLighting_1::IlluminationMode",[=[Lambert]=])
mmSetParamValue("::Group_2::LocalLighting_1::AmbientColor",[=[#ffffff]=])
mmSetParamValue("::Group_2::LocalLighting_1::DiffuseColor",[=[#ffffff]=])
mmSetParamValue("::Group_2::LocalLighting_1::SpecularColor",[=[#ffffff]=])
mmSetParamValue("::Group_2::LocalLighting_1::AmbientFactor",[=[0.200000]=])
mmSetParamValue("::Group_2::LocalLighting_1::DiffuseFactor",[=[0.700000]=])
mmSetParamValue("::Group_2::LocalLighting_1::SpecularFactor",[=[0.100000]=])
mmSetParamValue("::Group_2::LocalLighting_1::ExponentialFactor",[=[120.000000]=])
mmSetParamValue("::Group_2::LocalLighting_1::ExposureAvgIntensity",[=[0.500000]=])
mmSetParamValue("::Group_2::LocalLighting_1::Roughness",[=[0.700000]=])
mmSetParamValue("::Group_2::LocalLighting_1::OutputFormat",[=[GL_RGBA8_SNORM]=])
mmSetParamValue("::OpticalFlow_1::Offset",[=[1]=])
mmSetParamValue("::OpticalFlow_1::Framerate-Adjust",[=[3]=])
mmSetParamValue("::OpticalFlow_1::WindowSize",[=[2]=])
mmSetParamValue("::Group_2::AmbientLight_1::Intensity",[=[1.000000]=])
mmSetParamValue("::Group_2::AmbientLight_1::Color",[=[#cccccc]=])
mmSetParamValue("::MotionBlur_1::MaxBlurRadius",[=[6]=])
mmSetParamValue("::MotionBlur_1::NumSamples",[=[15]=])
mmSetParamValue("::MotionBlur_1::ExposureTime",[=[0.077000]=])
mmSetParamValue("::MotionBlur_1::FrameRate",[=[61]=])

mmSetGUIVisible(true)
mmSetGUIScale(1.000000)
mmSetGUIState([=[{"AnimationEditor_State":{"animation_bounds":[0,100],"animation_file":"","animations":null,"compact_view":false,"current_frame":0,"orient_source_index":-1,"output_prefix":"","playback_fps":30,"playing":0,"pos_source_index":-1,"write_to_graph":false},"ConfiguratorState":{"module_list_sidebar_width":250.0,"show_module_list_sidebar":false},"GUIState":{"font_file_name":"Roboto-Regular.ttf","font_size":12.0,"global_win_background_alpha":1.0,"imgui_settings":"[Window][Configurator     F11]\nPos=481,18\nSize=1439,762\nCollapsed=0\nDockId=0x00000004,0\n\n[Window][Parameters     F10]\nPos=0,18\nSize=479,762\nCollapsed=0\nDockId=0x00000003,0\n\n[Window][Log Console     F9]\nPos=0,782\nSize=1920,261\nCollapsed=0\nDockId=0x00000002,0\n\n[Window][WindowOverViewport_11111111]\nPos=0,18\nSize=1920,1025\nCollapsed=0\n\n[Window][Debug##Default]\nPos=60,60\nSize=400,400\nCollapsed=0\n\n[Window][###fbw1264846156]\nPos=760,271\nSize=400,500\nCollapsed=0\n\n[Window][###fbw1320885130]\nPos=760,271\nSize=400,500\nCollapsed=0\n\n[Docking][Data]\nDockSpace     ID=0x7C6B3D9B Window=0xA87D555D Pos=0,18 Size=1920,1025 Split=Y\n  DockNode    ID=0x00000001 Parent=0x7C6B3D9B SizeRef=1920,780 Split=X\n    DockNode  ID=0x00000003 Parent=0x00000001 SizeRef=479,780 Selected=0x20D298EF\n    DockNode  ID=0x00000004 Parent=0x00000001 SizeRef=1439,780 CentralNode=1 Selected=0xB6EA6B89\n  DockNode    ID=0x00000002 Parent=0x7C6B3D9B SizeRef=1920,261 Selected=0xF54B1F54\n\n","menu_visible":true,"style":2},"GraphStates":{"Project":{"Interfaces":{"Group_1":{"interface_slot_0":["::Group_1::SimpleRenderTarget_1::Color"],"interface_slot_1":["::Group_1::SimpleRenderTarget_1::Normals"],"interface_slot_2":["::Group_1::SimpleRenderTarget_1::Depth"],"interface_slot_3":["::Group_1::SimpleRenderTarget_1::Camera"],"interface_slot_4":["::Group_1::SimpleRenderTarget_1::rendering"]},"Group_2":{"interface_slot_5":["::Group_2::LocalLighting_1::OutputTexture"],"interface_slot_6":["::Group_2::LocalLighting_1::AlbedoTexture"],"interface_slot_7":["::Group_2::LocalLighting_1::NormalTexture"],"interface_slot_8":["::Group_2::LocalLighting_1::DepthTexture"],"interface_slot_9":["::Group_2::LocalLighting_1::Camera"]}},"Modules":{"::DrawToScreen_1":{"graph_position":[285.0,64.0]},"::Group_1::MoleculeBallifier_1":{"graph_position":[2208.0,112.0]},"::Group_1::PDBLoader_1":{"graph_position":[2467.0,112.0]},"::Group_1::SimpleRenderTarget_1":{"graph_position":[1673.0,112.0]},"::Group_1::SphereRenderer_1":{"graph_position":[1940.0,112.0]},"::Group_2::AmbientLight_1":{"graph_position":[1384.0,112.0]},"::Group_2::DistantLight_1":{"graph_position":[1190.0,112.0]},"::Group_2::LocalLighting_1":{"graph_position":[1032.0,232.0]},"::MotionBlur_1":{"graph_position":[539.0,238.0]},"::OpticalFlow_1":{"graph_position":[740.0,64.0]},"::View3DGL_1":{"graph_position":[64.0,64.0]}},"call_coloring_map":0,"call_coloring_mode":0,"canvas_scrolling":[31.0,75.0],"canvas_zooming":1.0,"param_extended_mode":false,"parameter_sidebar_width":300.0,"profiling_bar_height":300.0,"project_name":"Project_1","show_call_label":true,"show_call_slots_label":false,"show_grid":false,"show_module_label":true,"show_parameter_sidebar":false,"show_profiling_bar":false,"show_slot_label":false}},"ParameterStates":{"::Group_1::MoleculeBallifier_1::colorTableFilename":{"gui_highlight":false,"gui_presentation_mode":16,"gui_read-only":false,"gui_visibility":true},"::Group_1::MoleculeBallifier_1::colorWeight":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::MoleculeBallifier_1::coloringMode0":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::MoleculeBallifier_1::coloringMode1":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::MoleculeBallifier_1::maxGradColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::Group_1::MoleculeBallifier_1::midGradColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::Group_1::MoleculeBallifier_1::minGradColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::Group_1::MoleculeBallifier_1::specialColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::Group_1::PDBLoader_1::calcBBoxPerFrame":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::PDBLoader_1::calculateBonds":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::PDBLoader_1::capFilename":{"gui_highlight":false,"gui_presentation_mode":16,"gui_read-only":false,"gui_visibility":true},"::Group_1::PDBLoader_1::maxFrames":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::PDBLoader_1::pdbFilename":{"gui_highlight":false,"gui_presentation_mode":16,"gui_read-only":false,"gui_visibility":true},"::Group_1::PDBLoader_1::recomputeSTRIDEeachFrame":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::PDBLoader_1::solventResidues":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::PDBLoader_1::strideFlag":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::PDBLoader_1::xtcFilename":{"gui_highlight":false,"gui_presentation_mode":16,"gui_read-only":false,"gui_visibility":true},"::Group_1::SimpleRenderTarget_1::color Out Format":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SimpleRenderTarget_1::normals Out Format":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::ambient occlusion::apex":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::ambient occlusion::coneLength":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::ambient occlusion::enableLighting":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::ambient occlusion::highPrecisionTexture":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::ambient occlusion::offset":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::ambient occlusion::strength":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::ambient occlusion::useGsProxies":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::ambient occlusion::volumeSize":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::flag storage::selectedColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":false},"::Group_1::SphereRenderer_1::flag storage::softSelectedColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":false},"::Group_1::SphereRenderer_1::forceTime":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::outline::width":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_1::SphereRenderer_1::renderMode":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::scaling":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::shaderMode":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::SphereRenderer_1::splat::alphaScaling":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_1::SphereRenderer_1::splat::attenuateSubpixel":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_1::SphereRenderer_1::ssbo::staticData":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_1::SphereRenderer_1::useLocalBbox":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::AmbientLight_1::Color":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::Group_2::AmbientLight_1::Intensity":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::DistantLight_1::AngularDiameter":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::DistantLight_1::Color":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::Group_2::DistantLight_1::Direction":{"gui_highlight":false,"gui_presentation_mode":512,"gui_read-only":false,"gui_visibility":true},"::Group_2::DistantLight_1::EyeDirection":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::DistantLight_1::Intensity":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::LocalLighting_1::AmbientColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":false},"::Group_2::LocalLighting_1::AmbientFactor":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::LocalLighting_1::DiffuseColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":false},"::Group_2::LocalLighting_1::DiffuseFactor":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::LocalLighting_1::ExponentialFactor":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::LocalLighting_1::ExposureAvgIntensity":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::LocalLighting_1::IlluminationMode":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::LocalLighting_1::OutputFormat":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::LocalLighting_1::Roughness":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::LocalLighting_1::SpecularColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":false},"::Group_2::LocalLighting_1::SpecularFactor":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::MotionBlur_1::ExposureTime":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::MotionBlur_1::FrameRate":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::MotionBlur_1::MaxBlurRadius":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::MotionBlur_1::NumSamples":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::OpticalFlow_1::Framerate-Adjust":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::OpticalFlow_1::Offset":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::OpticalFlow_1::WindowSize":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::ParameterGroup::anim":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::ParameterGroup::view":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::anim::SpeedDown":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::anim::SpeedUp":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::anim::play":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::anim::speed":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::anim::time":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::anim::togglePlay":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::backCol":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::cam::farplane":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::cam::halfaperturedegrees":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::cam::nearplane":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::cam::orientation":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::cam::position":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::cam::projectiontype":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::camstore::restorecam":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::camstore::settings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::camstore::storecam":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::multicam::autoLoadSettings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::multicam::autoSaveSettings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::multicam::loadSettings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::multicam::overrideSettings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::multicam::saveSettings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::resetViewOnBBoxChange":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::showLookAt":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::view::cubeOrientation":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":true,"gui_visibility":false},"::View3DGL_1::view::defaultOrientation":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::view::defaultView":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::view::resetView":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::view::showViewCube":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::viewKey::AngleStep":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::viewKey::FixToWorldUp":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::viewKey::MouseSensitivity":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::viewKey::MoveStep":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::viewKey::RotPoint":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::View3DGL_1::viewKey::RunFactor":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true}},"WindowConfigurations":{"Animation Editor":{"win_callback":8,"win_collapsed":false,"win_flags":66560,"win_hotkey":[294,0],"win_position":[0.0,0.0],"win_reset_position":[0.0,0.0],"win_reset_size":[1600.0,800.0],"win_show":false,"win_size":[1600.0,800.0]},"Configurator":{"win_callback":6,"win_collapsed":false,"win_flags":66568,"win_hotkey":[300,0],"win_position":[481.0,18.0],"win_reset_position":[0.0,0.0],"win_reset_size":[750.0,500.0],"win_show":false,"win_size":[1439.0,762.0]},"Hotkey Editor":{"hotkey_list":[[{"key":293,"mods":4,"name":"_hotkey_gui_exit","parent":"","parent_type":2}],[{"key":83,"mods":2,"name":"_hotkey_gui_save_project","parent":"","parent_type":2}],[{"key":76,"mods":2,"name":"_hotkey_gui_load_project","parent":"","parent_type":2}],[{"key":301,"mods":0,"name":"_hotkey_gui_menu","parent":"","parent_type":2}],[{"key":292,"mods":0,"name":"_hotkey_gui_toggle_graph_entry","parent":"","parent_type":2}],[{"key":291,"mods":0,"name":"_hotkey_gui_trigger_screenshot","parent":"","parent_type":2}],[{"key":71,"mods":2,"name":"_hotkey_gui_show-hide","parent":"","parent_type":2}],[{"key":300,"mods":0,"name":"_hotkey_gui_window_Configurator","parent":"4881575061537038212","parent_type":3}],[{"key":77,"mods":3,"name":"_hotkey_gui_configurator_module_search","parent":"","parent_type":4}],[{"key":80,"mods":3,"name":"_hotkey_gui_configurator_param_search","parent":"","parent_type":4}],[{"key":261,"mods":0,"name":"_hotkey_gui_configurator_delete_graph_entry","parent":"","parent_type":4}],[{"key":83,"mods":3,"name":"_hotkey_gui_configurator_save_project","parent":"","parent_type":4}],[{"key":82,"mods":3,"name":"_hotkey_gui_configurator_layout_graph","parent":"","parent_type":4}],[{"key":299,"mods":0,"name":"_hotkey_gui_window_Parameters","parent":"18024257541452379355","parent_type":3}],[{"key":80,"mods":2,"name":"_hotkey_gui_parameterlist_param_search","parent":"","parent_type":4}],[{"key":298,"mods":0,"name":"_hotkey_gui_window_Log Console","parent":"5429376390396994821","parent_type":3}],[{"key":297,"mods":0,"name":"_hotkey_gui_window_Transfer Function Editor","parent":"17444004971698983282","parent_type":3}],[{"key":296,"mods":0,"name":"_hotkey_gui_window_Performance Metrics","parent":"3726554825953697363","parent_type":3}],[{"key":295,"mods":0,"name":"_hotkey_gui_window_Hotkey Editor","parent":"3325494972324889135","parent_type":3}],[{"key":294,"mods":0,"name":"_hotkey_gui_window_Animation Editor","parent":"9338180494231019969","parent_type":3}],[{"key":-1,"mods":0,"name":"_hotkey_gui_animationeditor_play_animation","parent":"","parent_type":4}],[{"key":-1,"mods":0,"name":"_hotkey_gui_animationeditor_stop_animation","parent":"","parent_type":4}],[{"key":-1,"mods":0,"name":"_hotkey_gui_animationeditor_reverse_play_animation","parent":"","parent_type":4}],[{"key":-1,"mods":0,"name":"_hotkey_gui_animationeditor_move_to_animation_start","parent":"","parent_type":4}],[{"key":-1,"mods":0,"name":"_hotkey_gui_animationeditor_move_to_animation_end","parent":"","parent_type":4}],[{"key":67,"mods":5,"name":"::View3DGL_1_camstore::storecam","parent":"::View3DGL_1::camstore::storecam","parent_type":1}],[{"key":67,"mods":4,"name":"::View3DGL_1_camstore::restorecam","parent":"::View3DGL_1::camstore::restorecam","parent_type":1}],[{"key":-1,"mods":0,"name":"::View3DGL_1_multicam::saveSettings","parent":"::View3DGL_1::multicam::saveSettings","parent_type":1}],[{"key":-1,"mods":0,"name":"::View3DGL_1_multicam::loadSettings","parent":"::View3DGL_1::multicam::loadSettings","parent_type":1}],[{"key":268,"mods":0,"name":"::View3DGL_1_view::resetView","parent":"::View3DGL_1::view::resetView","parent_type":1}],[{"key":32,"mods":0,"name":"::View3DGL_1_anim::togglePlay","parent":"::View3DGL_1::anim::togglePlay","parent_type":1}],[{"key":77,"mods":0,"name":"::View3DGL_1_anim::SpeedUp","parent":"::View3DGL_1::anim::SpeedUp","parent_type":1}],[{"key":78,"mods":0,"name":"::View3DGL_1_anim::SpeedDown","parent":"::View3DGL_1::anim::SpeedDown","parent_type":1}]],"win_callback":4,"win_collapsed":false,"win_flags":65536,"win_hotkey":[295,0],"win_position":[0.0,0.0],"win_reset_position":[0.0,0.0],"win_reset_size":[0.0,0.0],"win_show":false,"win_size":[0.0,0.0]},"Log Console":{"log_force_open":true,"log_level":2,"win_callback":7,"win_collapsed":false,"win_flags":68608,"win_hotkey":[298,0],"win_position":[0.0,782.0],"win_reset_position":[0.0,0.0],"win_reset_size":[500.0,50.0],"win_show":true,"win_size":[1920.0,261.0]},"Parameters":{"param_extended_mode":false,"param_modules_list":[],"win_callback":1,"win_collapsed":false,"win_flags":65544,"win_hotkey":[299,0],"win_position":[0.0,18.0],"win_reset_position":[0.0,0.0],"win_reset_size":[400.0,500.0],"win_show":true,"win_size":[479.0,762.0]},"Performance Metrics":{"fpsms_max_value_count":20,"fpsms_mode":0,"fpsms_refresh_rate":2.0,"fpsms_show_options":false,"win_callback":3,"win_collapsed":false,"win_flags":589889,"win_hotkey":[296,0],"win_position":[0.0,0.0],"win_reset_position":[0.0,0.0],"win_reset_size":[0.0,0.0],"win_show":false,"win_size":[0.0,0.0]},"Transfer Function Editor":{"tfe_active_param":"","tfe_view_minimized":false,"tfe_view_vertical":false,"win_callback":5,"win_collapsed":false,"win_flags":65600,"win_hotkey":[297,0],"win_position":[0.0,0.0],"win_reset_position":[0.0,0.0],"win_reset_size":[0.0,0.0],"win_show":false,"win_size":[0.0,0.0]}}}]=])
