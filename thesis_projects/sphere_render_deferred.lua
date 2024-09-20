mmCheckVersion("326a9df827ac8027-dirty")
mmCreateView("GraphEntry_1","View3DGL","::Group_3::view")

mmCreateModule("BoundingBoxRenderer","::Group_3::bbox")
mmCreateModule("DistantLight","::Group_2::distantlight")
mmCreateModule("SphereRenderer","::Group_2::renderer")
mmCreateModule("TestSpheresDataSource","::Group_2::data")
mmCreateModule("SimpleRenderTarget","::Group_2::SimpleRenderTarget_1")
mmCreateModule("LocalLighting","::Group_1::LocalLighting_1")
mmCreateModule("AmbientLight","::Group_1::AmbientLight_1")
mmCreateModule("DrawToScreen","::Group_3::DrawToScreen_1")
mmCreateModule("DistantLight","::Group_1::DistantLight_1")
mmCreateModule("OpticalFlow","::Group_1::OpticalFlow_1")
mmCreateModule("MotionBlur","::Group_1::MotionBlur_1")

mmCreateCall("CallRender3DGL","::Group_3::view::rendering","::Group_3::bbox::rendering")
mmCreateCall("CallRender3DGL","::Group_3::bbox::chainRendering","::Group_3::DrawToScreen_1::rendering")
mmCreateCall("MultiParticleDataCall","::Group_2::renderer::getdata","::Group_2::data::getData")
mmCreateCall("CallLight","::Group_2::renderer::lights","::Group_2::distantlight::deployLightSlot")
mmCreateCall("CallRender3DGL","::Group_2::SimpleRenderTarget_1::chainRendering","::Group_2::renderer::rendering")
mmCreateCall("CallTexture2D","::Group_1::LocalLighting_1::AlbedoTexture","::Group_2::SimpleRenderTarget_1::Color")
mmCreateCall("CallTexture2D","::Group_1::LocalLighting_1::NormalTexture","::Group_2::SimpleRenderTarget_1::Normals")
mmCreateCall("CallTexture2D","::Group_1::LocalLighting_1::DepthTexture","::Group_2::SimpleRenderTarget_1::Depth")
mmCreateCall("CallLight","::Group_1::LocalLighting_1::lights","::Group_1::DistantLight_1::deployLightSlot")
mmCreateCall("CallCamera","::Group_1::LocalLighting_1::Camera","::Group_2::SimpleRenderTarget_1::Camera")
mmCreateCall("CallRender3DGL","::Group_3::DrawToScreen_1::chainRendering","::Group_2::SimpleRenderTarget_1::rendering")
mmCreateCall("CallTexture2D","::Group_3::DrawToScreen_1::InputTexture","::Group_1::MotionBlur_1::OutputTexture")
mmCreateCall("CallTexture2D","::Group_3::DrawToScreen_1::DepthTexture","::Group_2::SimpleRenderTarget_1::Depth")
mmCreateCall("CallLight","::Group_1::DistantLight_1::getLightSlot","::Group_1::AmbientLight_1::deployLightSlot")
mmCreateCall("CallTexture2D","::Group_1::OpticalFlow_1::InputTexture","::Group_1::LocalLighting_1::OutputTexture")
mmCreateCall("CallTexture2D","::Group_1::MotionBlur_1::ColorTexture","::Group_2::SimpleRenderTarget_1::Color")
mmCreateCall("CallTexture2D","::Group_1::MotionBlur_1::DepthTexture","::Group_2::SimpleRenderTarget_1::Depth")
mmCreateCall("CallTexture2D","::Group_1::MotionBlur_1::FlowTexture","::Group_1::OpticalFlow_1::FlowFieldTexture")

mmSetParamValue("::Group_3::view::camstore::settings",[=[]=])
mmSetParamValue("::Group_3::view::multicam::overrideSettings",[=[false]=])
mmSetParamValue("::Group_3::view::multicam::autoSaveSettings",[=[false]=])
mmSetParamValue("::Group_3::view::multicam::autoLoadSettings",[=[true]=])
mmSetParamValue("::Group_3::view::resetViewOnBBoxChange",[=[false]=])
mmSetParamValue("::Group_3::view::showLookAt",[=[false]=])
mmSetParamValue("::Group_3::view::view::showViewCube",[=[false]=])
mmSetParamValue("::Group_3::view::anim::play",[=[true]=])
mmSetParamValue("::Group_3::view::anim::speed",[=[60.000000]=])
mmSetParamValue("::Group_3::view::anim::time",[=[13.246826]=])
mmSetParamValue("::Group_3::view::backCol",[=[#000020]=])
mmSetParamValue("::Group_3::view::viewKey::MoveStep",[=[0.500000]=])
mmSetParamValue("::Group_3::view::viewKey::RunFactor",[=[2.000000]=])
mmSetParamValue("::Group_3::view::viewKey::AngleStep",[=[90.000000]=])
mmSetParamValue("::Group_3::view::viewKey::FixToWorldUp",[=[true]=])
mmSetParamValue("::Group_3::view::viewKey::MouseSensitivity",[=[3.000000]=])
mmSetParamValue("::Group_3::view::viewKey::RotPoint",[=[Look-At]=])
mmSetParamValue("::Group_3::view::view::cubeOrientation",[=[-0.0657457858;-0.0424370393;-0.00279890024;0.996929705]=])
mmSetParamValue("::Group_3::view::view::defaultView",[=[FACE - Front]=])
mmSetParamValue("::Group_3::view::view::defaultOrientation",[=[Top]=])
mmSetParamValue("::Group_3::view::cam::position",[=[-0.669874489;0.937961876;9.45604038]=])
mmSetParamValue("::Group_3::view::cam::orientation",[=[-0.0657457858;-0.0424370393;-0.00279890024;0.996929705]=])
mmSetParamValue("::Group_3::view::cam::projectiontype",[=[Perspective]=])
mmSetParamValue("::Group_3::view::cam::nearplane",[=[8.316525]=])
mmSetParamValue("::Group_3::view::cam::farplane",[=[10.723172]=])
mmSetParamValue("::Group_3::view::cam::halfaperturedegrees",[=[28.647890]=])
mmSetParamValue("::Group_3::bbox::enableBoundingBox",[=[true]=])
mmSetParamValue("::Group_3::bbox::boundingBoxColor",[=[#ffffff]=])
mmSetParamValue("::Group_3::bbox::smoothLines",[=[true]=])
mmSetParamValue("::Group_3::bbox::enableViewCube",[=[false]=])
mmSetParamValue("::Group_3::bbox::viewCubePosition",[=[top right]=])
mmSetParamValue("::Group_3::bbox::viewCubeSize",[=[100]=])
mmSetParamValue("::Group_2::distantlight::Intensity",[=[1.000000]=])
mmSetParamValue("::Group_2::distantlight::Color",[=[#cccccc]=])
mmSetParamValue("::Group_2::distantlight::Direction",[=[0.426968038;-0.352481544;-0.75396508]=])
mmSetParamValue("::Group_2::distantlight::AngularDiameter",[=[0.000000]=])
mmSetParamValue("::Group_2::distantlight::EyeDirection",[=[false]=])
mmSetParamValue("::Group_2::renderer::shaderMode",[=[Forward]=])
mmSetParamValue("::Group_2::renderer::scaling",[=[1.000000]=])
mmSetParamValue("::Group_2::renderer::forceTime",[=[false]=])
mmSetParamValue("::Group_2::renderer::useLocalBbox",[=[false]=])
mmSetParamValue("::Group_2::renderer::flag storage::selectedColor",[=[#ff0000]=])
mmSetParamValue("::Group_2::renderer::flag storage::softSelectedColor",[=[#ff8080]=])
mmSetParamValue("::Group_2::renderer::splat::alphaScaling",[=[5.000000]=])
mmSetParamValue("::Group_2::renderer::splat::attenuateSubpixel",[=[false]=])
mmSetParamValue("::Group_2::renderer::ssbo::staticData",[=[false]=])
mmSetParamValue("::Group_2::renderer::ambient occlusion::enableLighting",[=[false]=])
mmSetParamValue("::Group_2::renderer::ambient occlusion::useGsProxies",[=[false]=])
mmSetParamValue("::Group_2::renderer::ambient occlusion::volumeSize",[=[128]=])
mmSetParamValue("::Group_2::renderer::ambient occlusion::apex",[=[50.000000]=])
mmSetParamValue("::Group_2::renderer::ambient occlusion::offset",[=[0.010000]=])
mmSetParamValue("::Group_2::renderer::ambient occlusion::strength",[=[1.000000]=])
mmSetParamValue("::Group_2::renderer::ambient occlusion::coneLength",[=[0.800000]=])
mmSetParamValue("::Group_2::renderer::ambient occlusion::highPrecisionTexture",[=[false]=])
mmSetParamValue("::Group_2::renderer::outline::width",[=[2.000000]=])
mmSetParamValue("::Group_2::renderer::renderMode",[=[Simple]=])
mmSetParamValue("::Group_2::data::numSpheres",[=[15]=])
mmSetParamValue("::Group_2::data::numFrames",[=[100]=])
mmSetParamValue("::Group_2::SimpleRenderTarget_1::color Out Format",[=[GL_RGBA8_SNORM]=])
mmSetParamValue("::Group_2::SimpleRenderTarget_1::normals Out Format",[=[GL_RGB16F]=])
mmSetParamValue("::Group_1::LocalLighting_1::IlluminationMode",[=[Lambert]=])
mmSetParamValue("::Group_1::LocalLighting_1::AmbientColor",[=[#ffffff]=])
mmSetParamValue("::Group_1::LocalLighting_1::DiffuseColor",[=[#ffffff]=])
mmSetParamValue("::Group_1::LocalLighting_1::SpecularColor",[=[#ffffff]=])
mmSetParamValue("::Group_1::LocalLighting_1::AmbientFactor",[=[0.200000]=])
mmSetParamValue("::Group_1::LocalLighting_1::DiffuseFactor",[=[0.700000]=])
mmSetParamValue("::Group_1::LocalLighting_1::SpecularFactor",[=[0.100000]=])
mmSetParamValue("::Group_1::LocalLighting_1::ExponentialFactor",[=[120.000000]=])
mmSetParamValue("::Group_1::LocalLighting_1::ExposureAvgIntensity",[=[0.500000]=])
mmSetParamValue("::Group_1::LocalLighting_1::Roughness",[=[0.700000]=])
mmSetParamValue("::Group_1::LocalLighting_1::OutputFormat",[=[GL_RGBA8_SNORM]=])
mmSetParamValue("::Group_1::AmbientLight_1::Intensity",[=[1.000000]=])
mmSetParamValue("::Group_1::AmbientLight_1::Color",[=[#cccccc]=])
mmSetParamValue("::Group_1::DistantLight_1::Intensity",[=[1.000000]=])
mmSetParamValue("::Group_1::DistantLight_1::Color",[=[#cccccc]=])
mmSetParamValue("::Group_1::DistantLight_1::Direction",[=[-0.25;-0.5;-0.75]=])
mmSetParamValue("::Group_1::DistantLight_1::AngularDiameter",[=[0.000000]=])
mmSetParamValue("::Group_1::DistantLight_1::EyeDirection",[=[false]=])
mmSetParamValue("::Group_1::OpticalFlow_1::Lambda",[=[0.000000]=])
mmSetParamValue("::Group_1::OpticalFlow_1::Theta",[=[2]=])
mmSetParamValue("::Group_1::OpticalFlow_1::Framerate-Adjust",[=[18]=])
mmSetParamValue("::Group_1::MotionBlur_1::MaxBlurRadius",[=[11]=])
mmSetParamValue("::Group_1::MotionBlur_1::NumSamples",[=[15]=])
mmSetParamValue("::Group_1::MotionBlur_1::ExposureTime",[=[0.081000]=])
mmSetParamValue("::Group_1::MotionBlur_1::FrameRate",[=[76]=])

mmSetGUIVisible(true)
mmSetGUIScale(1.000000)
mmSetGUIState([=[{"AnimationEditor_State":{"animation_bounds":[0,100],"animation_file":"","animations":null,"compact_view":false,"current_frame":0,"orient_source_index":-1,"output_prefix":"","playback_fps":30,"playing":0,"pos_source_index":-1,"write_to_graph":false},"ConfiguratorState":{"module_list_sidebar_width":250.0,"show_module_list_sidebar":false},"GUIState":{"font_file_name":"Roboto-Regular.ttf","font_size":12.0,"global_win_background_alpha":1.0,"imgui_settings":"[Window][Configurator     F11]\nPos=481,18\nSize=1439,762\nCollapsed=0\nDockId=0x00000004,0\n\n[Window][Parameters     F10]\nPos=0,18\nSize=335,976\nCollapsed=0\nDockId=0x00000003,0\n\n[Window][Log Console     F9]\nPos=0,996\nSize=1920,47\nCollapsed=0\nDockId=0x00000002,0\n\n[Window][WindowOverViewport_11111111]\nPos=0,18\nSize=1920,1025\nCollapsed=0\n\n[Window][Debug##Default]\nPos=60,60\nSize=400,400\nCollapsed=0\n\n[Window][###fbw1358555268]\nPos=760,271\nSize=400,500\nCollapsed=0\n\n[Window][###fbw2086368611]\nPos=760,271\nSize=400,500\nCollapsed=0\n\n[Window][###fbw648194736]\nPos=760,290\nSize=400,500\nCollapsed=0\n\n[Window][###fbw94319336]\nPos=760,271\nSize=400,500\nCollapsed=0\n\n[Window][###fbw197029651]\nPos=760,290\nSize=400,500\nCollapsed=0\n\n[Window][###fbw344431991]\nPos=760,271\nSize=400,500\nCollapsed=0\n\n[Window][###fbw247372381]\nPos=760,271\nSize=400,500\nCollapsed=0\n\n[Window][###fbw2122460388]\nPos=760,270\nSize=400,500\nCollapsed=0\n\n[Window][###fbw188950178]\nPos=760,271\nSize=400,500\nCollapsed=0\n\n[Window][###fbw1605426261]\nPos=760,271\nSize=400,500\nCollapsed=0\n\n[Window][###fbw1936703145]\nPos=760,271\nSize=400,500\nCollapsed=0\n\n[Docking][Data]\nDockSpace     ID=0x7C6B3D9B Window=0xA87D555D Pos=0,18 Size=1920,1025 Split=Y\n  DockNode    ID=0x00000001 Parent=0x7C6B3D9B SizeRef=1920,976 Split=X\n    DockNode  ID=0x00000003 Parent=0x00000001 SizeRef=335,780 Selected=0x20D298EF\n    DockNode  ID=0x00000004 Parent=0x00000001 SizeRef=1583,780 CentralNode=1 Selected=0xB6EA6B89\n  DockNode    ID=0x00000002 Parent=0x7C6B3D9B SizeRef=1920,47 Selected=0xF54B1F54\n\n","menu_visible":true,"style":2},"GraphStates":{"Project":{"Interfaces":{"Group_1":{"interface_slot_10":["::Group_1::LocalLighting_1::AlbedoTexture"],"interface_slot_11":["::Group_1::LocalLighting_1::NormalTexture"],"interface_slot_12":["::Group_1::LocalLighting_1::DepthTexture"],"interface_slot_13":["::Group_1::LocalLighting_1::Camera"],"interface_slot_14":["::Group_1::MotionBlur_1::ColorTexture"],"interface_slot_15":["::Group_1::MotionBlur_1::DepthTexture"],"interface_slot_8":["::Group_1::MotionBlur_1::OutputTexture"],"interface_slot_9":["::Group_1::OpticalFlow_1::FlowFieldTexture"]},"Group_2":{"interface_slot_3":["::Group_2::SimpleRenderTarget_1::Color"],"interface_slot_4":["::Group_2::SimpleRenderTarget_1::Normals"],"interface_slot_5":["::Group_2::SimpleRenderTarget_1::Depth"],"interface_slot_6":["::Group_2::SimpleRenderTarget_1::Camera"],"interface_slot_7":["::Group_2::SimpleRenderTarget_1::rendering"]},"Group_3":{"interface_slot_0":["::Group_3::DrawToScreen_1::chainRendering"],"interface_slot_1":["::Group_3::DrawToScreen_1::InputTexture"],"interface_slot_2":["::Group_3::DrawToScreen_1::DepthTexture"]}},"Modules":{"::Group_1::AmbientLight_1":{"graph_position":[1071.6033935546875,573.46142578125]},"::Group_1::DistantLight_1":{"graph_position":[940.373779296875,568.495849609375]},"::Group_1::LocalLighting_1":{"graph_position":[791.9810791015625,517.1959228515625]},"::Group_1::MotionBlur_1":{"graph_position":[392.8340148925781,668.4410400390625]},"::Group_1::OpticalFlow_1":{"graph_position":[541.4526977539063,562.8787841796875]},"::Group_2::SimpleRenderTarget_1":{"graph_position":[1432.6624755859375,303.0208740234375]},"::Group_2::data":{"graph_position":[1876.3175048828125,288.97149658203125]},"::Group_2::distantlight":{"graph_position":[1876.3175048828125,384.9715576171875]},"::Group_2::renderer":{"graph_position":[1619.317626953125,288.97149658203125]},"::Group_3::DrawToScreen_1":{"graph_position":[94.96810913085938,170.30511474609375]},"::Group_3::bbox":{"graph_position":[-95.44862365722656,173.25881958007813]},"::Group_3::view":{"graph_position":[-229.76278686523438,167.24288940429688]}},"call_coloring_map":0,"call_coloring_mode":0,"canvas_scrolling":[66.35515594482422,12.83248519897461],"canvas_zooming":0.6595615148544312,"param_extended_mode":false,"parameter_sidebar_width":300.0,"profiling_bar_height":300.0,"project_name":"Project_1","show_call_label":true,"show_call_slots_label":false,"show_grid":false,"show_module_label":true,"show_parameter_sidebar":false,"show_profiling_bar":false,"show_slot_label":false}},"ParameterStates":{"::Group_1::AmbientLight_1::Color":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::Group_1::AmbientLight_1::Intensity":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::DistantLight_1::AngularDiameter":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::DistantLight_1::Color":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::Group_1::DistantLight_1::Direction":{"gui_highlight":false,"gui_presentation_mode":512,"gui_read-only":false,"gui_visibility":true},"::Group_1::DistantLight_1::EyeDirection":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::DistantLight_1::Intensity":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::LocalLighting_1::AmbientColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":false},"::Group_1::LocalLighting_1::AmbientFactor":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_1::LocalLighting_1::DiffuseColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":false},"::Group_1::LocalLighting_1::DiffuseFactor":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_1::LocalLighting_1::ExponentialFactor":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_1::LocalLighting_1::ExposureAvgIntensity":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::LocalLighting_1::IlluminationMode":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::LocalLighting_1::OutputFormat":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::LocalLighting_1::Roughness":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::LocalLighting_1::SpecularColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":false},"::Group_1::LocalLighting_1::SpecularFactor":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_1::MotionBlur_1::ExposureTime":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::MotionBlur_1::FrameRate":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::MotionBlur_1::MaxBlurRadius":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::MotionBlur_1::NumSamples":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::OpticalFlow_1::Framerate-Adjust":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::OpticalFlow_1::Lambda":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_1::OpticalFlow_1::Theta":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::SimpleRenderTarget_1::color Out Format":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::SimpleRenderTarget_1::normals Out Format":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::data::numFrames":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::data::numSpheres":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::distantlight::AngularDiameter":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::distantlight::Color":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::Group_2::distantlight::Direction":{"gui_highlight":false,"gui_presentation_mode":512,"gui_read-only":false,"gui_visibility":true},"::Group_2::distantlight::EyeDirection":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::distantlight::Intensity":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::renderer::ambient occlusion::apex":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::ambient occlusion::coneLength":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::ambient occlusion::enableLighting":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::ambient occlusion::highPrecisionTexture":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::ambient occlusion::offset":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::ambient occlusion::strength":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::ambient occlusion::useGsProxies":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::ambient occlusion::volumeSize":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::flag storage::selectedColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::flag storage::softSelectedColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::forceTime":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::renderer::outline::width":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::renderMode":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::renderer::scaling":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::renderer::shaderMode":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_2::renderer::splat::alphaScaling":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::splat::attenuateSubpixel":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::ssbo::staticData":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":false},"::Group_2::renderer::useLocalBbox":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::bbox::boundingBoxColor":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::Group_3::bbox::enableBoundingBox":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::bbox::enableViewCube":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::bbox::smoothLines":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::bbox::viewCubePosition":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::bbox::viewCubeSize":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::ParameterGroup::anim":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::ParameterGroup::view":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::anim::SpeedDown":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::anim::SpeedUp":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::anim::play":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::anim::speed":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::anim::time":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::anim::togglePlay":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::backCol":{"gui_highlight":false,"gui_presentation_mode":8,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::cam::farplane":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::cam::halfaperturedegrees":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::cam::nearplane":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::cam::orientation":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::cam::position":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::cam::projectiontype":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::camstore::restorecam":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::camstore::settings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::camstore::storecam":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::multicam::autoLoadSettings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::multicam::autoSaveSettings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::multicam::loadSettings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::multicam::overrideSettings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::multicam::saveSettings":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::resetViewOnBBoxChange":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::showLookAt":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::view::cubeOrientation":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":true,"gui_visibility":false},"::Group_3::view::view::defaultOrientation":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::view::defaultView":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::view::resetView":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::view::showViewCube":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::viewKey::AngleStep":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::viewKey::FixToWorldUp":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::viewKey::MouseSensitivity":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::viewKey::MoveStep":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::viewKey::RotPoint":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true},"::Group_3::view::viewKey::RunFactor":{"gui_highlight":false,"gui_presentation_mode":2,"gui_read-only":false,"gui_visibility":true}},"WindowConfigurations":{"Animation Editor":{"win_callback":8,"win_collapsed":false,"win_flags":66560,"win_hotkey":[294,0],"win_position":[0.0,0.0],"win_reset_position":[0.0,0.0],"win_reset_size":[1600.0,800.0],"win_show":false,"win_size":[1600.0,800.0]},"Configurator":{"win_callback":6,"win_collapsed":false,"win_flags":66568,"win_hotkey":[300,0],"win_position":[481.0,18.0],"win_reset_position":[0.0,0.0],"win_reset_size":[750.0,500.0],"win_show":false,"win_size":[1439.0,762.0]},"Hotkey Editor":{"hotkey_list":[[{"key":293,"mods":4,"name":"_hotkey_gui_exit","parent":"","parent_type":2}],[{"key":83,"mods":2,"name":"_hotkey_gui_save_project","parent":"","parent_type":2}],[{"key":76,"mods":2,"name":"_hotkey_gui_load_project","parent":"","parent_type":2}],[{"key":301,"mods":0,"name":"_hotkey_gui_menu","parent":"","parent_type":2}],[{"key":292,"mods":0,"name":"_hotkey_gui_toggle_graph_entry","parent":"","parent_type":2}],[{"key":291,"mods":0,"name":"_hotkey_gui_trigger_screenshot","parent":"","parent_type":2}],[{"key":71,"mods":2,"name":"_hotkey_gui_show-hide","parent":"","parent_type":2}],[{"key":300,"mods":0,"name":"_hotkey_gui_window_Configurator","parent":"4881575061537038212","parent_type":3}],[{"key":77,"mods":3,"name":"_hotkey_gui_configurator_module_search","parent":"","parent_type":4}],[{"key":80,"mods":3,"name":"_hotkey_gui_configurator_param_search","parent":"","parent_type":4}],[{"key":261,"mods":0,"name":"_hotkey_gui_configurator_delete_graph_entry","parent":"","parent_type":4}],[{"key":83,"mods":3,"name":"_hotkey_gui_configurator_save_project","parent":"","parent_type":4}],[{"key":82,"mods":3,"name":"_hotkey_gui_configurator_layout_graph","parent":"","parent_type":4}],[{"key":299,"mods":0,"name":"_hotkey_gui_window_Parameters","parent":"18024257541452379355","parent_type":3}],[{"key":80,"mods":2,"name":"_hotkey_gui_parameterlist_param_search","parent":"","parent_type":4}],[{"key":298,"mods":0,"name":"_hotkey_gui_window_Log Console","parent":"5429376390396994821","parent_type":3}],[{"key":297,"mods":0,"name":"_hotkey_gui_window_Transfer Function Editor","parent":"17444004971698983282","parent_type":3}],[{"key":296,"mods":0,"name":"_hotkey_gui_window_Performance Metrics","parent":"3726554825953697363","parent_type":3}],[{"key":295,"mods":0,"name":"_hotkey_gui_window_Hotkey Editor","parent":"3325494972324889135","parent_type":3}],[{"key":294,"mods":0,"name":"_hotkey_gui_window_Animation Editor","parent":"9338180494231019969","parent_type":3}],[{"key":-1,"mods":0,"name":"_hotkey_gui_animationeditor_play_animation","parent":"","parent_type":4}],[{"key":-1,"mods":0,"name":"_hotkey_gui_animationeditor_stop_animation","parent":"","parent_type":4}],[{"key":-1,"mods":0,"name":"_hotkey_gui_animationeditor_reverse_play_animation","parent":"","parent_type":4}],[{"key":-1,"mods":0,"name":"_hotkey_gui_animationeditor_move_to_animation_start","parent":"","parent_type":4}],[{"key":-1,"mods":0,"name":"_hotkey_gui_animationeditor_move_to_animation_end","parent":"","parent_type":4}],[{"key":67,"mods":5,"name":"::Group_3::view_camstore::storecam","parent":"::Group_3::view::camstore::storecam","parent_type":1}],[{"key":67,"mods":4,"name":"::Group_3::view_camstore::restorecam","parent":"::Group_3::view::camstore::restorecam","parent_type":1}],[{"key":-1,"mods":0,"name":"::Group_3::view_multicam::saveSettings","parent":"::Group_3::view::multicam::saveSettings","parent_type":1}],[{"key":-1,"mods":0,"name":"::Group_3::view_multicam::loadSettings","parent":"::Group_3::view::multicam::loadSettings","parent_type":1}],[{"key":268,"mods":0,"name":"::Group_3::view_view::resetView","parent":"::Group_3::view::view::resetView","parent_type":1}],[{"key":32,"mods":0,"name":"::Group_3::view_anim::togglePlay","parent":"::Group_3::view::anim::togglePlay","parent_type":1}],[{"key":77,"mods":0,"name":"::Group_3::view_anim::SpeedUp","parent":"::Group_3::view::anim::SpeedUp","parent_type":1}],[{"key":78,"mods":0,"name":"::Group_3::view_anim::SpeedDown","parent":"::Group_3::view::anim::SpeedDown","parent_type":1}]],"win_callback":4,"win_collapsed":false,"win_flags":65536,"win_hotkey":[295,0],"win_position":[0.0,0.0],"win_reset_position":[0.0,0.0],"win_reset_size":[0.0,0.0],"win_show":false,"win_size":[0.0,0.0]},"Log Console":{"log_force_open":true,"log_level":2,"win_callback":7,"win_collapsed":false,"win_flags":68608,"win_hotkey":[298,0],"win_position":[0.0,996.0],"win_reset_position":[0.0,0.0],"win_reset_size":[500.0,50.0],"win_show":true,"win_size":[1920.0,47.0]},"Parameters":{"param_extended_mode":false,"param_modules_list":[],"win_callback":1,"win_collapsed":false,"win_flags":65544,"win_hotkey":[299,0],"win_position":[0.0,18.0],"win_reset_position":[0.0,0.0],"win_reset_size":[400.0,500.0],"win_show":true,"win_size":[335.0,976.0]},"Performance Metrics":{"fpsms_max_value_count":20,"fpsms_mode":0,"fpsms_refresh_rate":2.0,"fpsms_show_options":false,"win_callback":3,"win_collapsed":false,"win_flags":589889,"win_hotkey":[296,0],"win_position":[0.0,0.0],"win_reset_position":[0.0,0.0],"win_reset_size":[0.0,0.0],"win_show":false,"win_size":[0.0,0.0]},"Transfer Function Editor":{"tfe_active_param":"","tfe_view_minimized":false,"tfe_view_vertical":false,"win_callback":5,"win_collapsed":false,"win_flags":65600,"win_hotkey":[297,0],"win_position":[0.0,0.0],"win_reset_position":[0.0,0.0],"win_reset_size":[0.0,0.0],"win_show":false,"win_size":[0.0,0.0]}}}]=])
