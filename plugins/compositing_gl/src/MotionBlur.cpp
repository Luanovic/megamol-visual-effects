#include "DepthPeelingRenderTarget.h"

#include "compositing_gl/CompositingCalls.h"

megamol::compositing_gl::DepthPeelingRenderTarget::DepthPeelingRenderTarget()
        : core::view::RendererModule<mmstd_gl::CallRender3DGL, ModuleGL>()
        , m_version(0)
        , m_GBuffer(nullptr)
        , blur_strength_slot_("blurStrength", "Controls the strength of the motion blur effect")
        , sample_count_slot_("sampleCount", "Sets the number of samples for the blur effect") {
        , m_motion_blur_prgrm(nullptr)
{
    this->m_color_render_target.SetCallback(
        CallTexture2D::ClassName(), "GetData", &DepthPeelingRenderTarget::getColorRenderTarget);
    this->m_color_render_target.SetCallback(
        CallTexture2D::ClassName(), "GetMetaData", &DepthPeelingRenderTarget::getMetaDataCallback);
    this->MakeSlotAvailable(&this->m_color_render_target);

    this->m_normal_render_target.SetCallback(
        CallTexture2D::ClassName(), "GetData", &DepthPeelingRenderTarget::getNormalsRenderTarget);
    this->m_normal_render_target.SetCallback(
        CallTexture2D::ClassName(), "GetMetaData", &DepthPeelingRenderTarget::getMetaDataCallback);
    this->MakeSlotAvailable(&this->m_normal_render_target);

    this->m_depth_buffer.SetCallback(CallTexture2D::ClassName(), "GetData", &DepthPeelingRenderTarget::getDepthBuffer);
    this->m_depth_buffer.SetCallback(
        CallTexture2D::ClassName(), "GetMetaData", &DepthPeelingRenderTarget::getMetaDataCallback);
    this->MakeSlotAvailable(&this->m_depth_buffer);

    this->m_camera.SetCallback(CallCamera::ClassName(), "GetData", &DepthPeelingRenderTarget::getCameraSnapshot);
    this->m_camera.SetCallback(CallCamera::ClassName(), "GetMetaData", &DepthPeelingRenderTarget::getMetaDataCallback);
    this->MakeSlotAvailable(&this->m_camera);

    layers.SetParameter(new core::param::IntParam(4, 1, 10));
    this->MakeSlotAvailable(&layers);
    layers.ForceSetDirty();

    this->MakeSlotAvailable(&this->m_framebuffer_slot);

    this->MakeSlotAvailable(&this->chainRenderSlot);
    this->MakeSlotAvailable(&this->renderSlot);

    this->MakeSlotAvailable(colorOutHandler_.getFormatSelectorSlot());

    this->MakeSlotAvailable(normalsOutHandler_.getFormatSelectorSlot());
}

megamol::compositing_gl::DepthPeelingRenderTarget::~DepthPeelingRenderTarget() {
    m_GBuffer.reset();
    this->Release();
}

bool megamol::compositing_gl::DepthPeelingRenderTarget::create() {

    m_GBuffer = std::make_shared<glowl::FramebufferObject>(1, 1);
    m_GBuffer->createColorAttachment(colorOutHandler_.getInternalFormat(), colorOutHandler_.getFormat(),
        colorOutHandler_.getType()); // surface albedo
    m_GBuffer->createColorAttachment(normalsOutHandler_.getInternalFormat(), normalsOutHandler_.getFormat(),
        normalsOutHandler_.getType()); // normals

    return true;
}

void megamol::compositing_gl::DepthPeelingRenderTarget::release() {}

bool megamol::compositing_gl::DepthPeelingRenderTarget::GetExtents(mmstd_gl::CallRender3DGL& call) {
    mmstd_gl::CallRender3DGL* chainedCall = this->chainRenderSlot.CallAs<mmstd_gl::CallRender3DGL>();
    if (chainedCall != nullptr) {
        *chainedCall = call;
        bool retVal = (*chainedCall)(core::view::AbstractCallRender::FnGetExtents);
        call = *chainedCall;
    }
    return true;
}

bool megamol::compositing_gl::DepthPeelingRenderTarget::Render(mmstd_gl::CallRender3DGL& call) {
    ++m_version;

    m_last_used_camera = call.GetCamera();

    auto call_fbo = call.GetFramebuffer();

    if (m_GBuffer->getWidth() != call_fbo->getWidth() || m_GBuffer->getHeight() != call_fbo->getHeight()) {
        m_GBuffer->resize(call_fbo->getWidth(), call_fbo->getHeight());
    }

    // this framebuffer will use 0 clear color because it uses alpha transparency during
    // compositing and final presentation to screen anyway
    m_GBuffer->bind();
    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mmstd_gl::CallRender3DGL* chained_call = this->chainRenderSlot.CallAs<mmstd_gl::CallRender3DGL>();
    if (chained_call != nullptr) {
        *chained_call = call;

        chained_call->SetFramebuffer(m_GBuffer);

        if (!((*chained_call)(core::view::AbstractCallRender::FnRender))) {
            return false;
        }
    }

    return true;
}

bool megamol::compositing_gl::DepthPeelingRenderTarget::getColorRenderTarget(core::Call& caller) {
    auto ct = dynamic_cast<CallTexture2D*>(&caller);

    if (ct == NULL)
        return false;

    ct->setData(m_GBuffer->getColorAttachment(0), m_version);

    return true;
}

bool megamol::compositing_gl::DepthPeelingRenderTarget::getNormalsRenderTarget(core::Call& caller) {
    auto ct = dynamic_cast<CallTexture2D*>(&caller);

    if (ct == NULL)
        return false;

    ct->setData(m_GBuffer->getColorAttachment(1), m_version);

    return true;
}

bool megamol::compositing_gl::DepthPeelingRenderTarget::getDepthBuffer(core::Call& caller) {
    auto ct = dynamic_cast<CallTexture2D*>(&caller);

    if (ct == NULL)
        return false;

    ct->setData(m_GBuffer->getDepthStencil(), m_version);

    return true;
}

bool megamol::compositing_gl::DepthPeelingRenderTarget::getCameraSnapshot(core::Call& caller) {
    auto cc = dynamic_cast<CallCamera*>(&caller);

    if (cc == NULL)
        return false;

    cc->setData(m_last_used_camera, m_version);

    return true;
}

bool megamol::compositing_gl::DepthPeelingRenderTarget::getFramebufferObject(core::Call& caller) {
    auto cf = dynamic_cast<CallFramebufferGL*>(&caller);

    if (cf == NULL)
        return false;

    cf->setData(m_GBuffer, m_version);

    return true;
}

bool megamol::compositing_gl::DepthPeelingRenderTarget::getMetaDataCallback(core::Call& caller) {
    return true;
}

bool megamol::compositing_gl::DepthPeelingRenderTarget::textureFormatUpdate() {
    /*
    if (m_GBuffer == NULL) {
        m_GBuffer = std::make_shared<glowl::FramebufferObject>(1, 1);
        m_GBuffer->createColorAttachment(colorOutHandler_.getInternalFormat(), colorOutHandler_.getFormat(), colorOutHandler_.getType()); // surface albedo
        m_GBuffer->createColorAttachment(normalsOutHandler_.getInternalFormat(), normalsOutHandler_.getFormat(), normalsOutHandler_.getType());   // normals
    } else {
        glowl::TextureLayout col_tx_layout(
            colorOutHandler_.getInternalFormat(), 1, 1, 1, colorOutHandler_.getFormat(), colorOutHandler_.getType(), 1);
        m_GBuffer->getColorAttachment(0)->reload(col_tx_layout, nullptr);
        glowl::TextureLayout norm_tx_layout(normalsOutHandler_.getInternalFormat(), 1, 1, 1,
            normalsOutHandler_.getFormat(), normalsOutHandler_.getType(), 1);
        m_GBuffer->getColorAttachment(1)->reload(norm_tx_layout, nullptr);
    }
    */

    m_GBuffer.reset();
    m_GBuffer = std::make_shared<glowl::FramebufferObject>(1, 1);
    m_GBuffer->createColorAttachment(colorOutHandler_.getInternalFormat(), colorOutHandler_.getFormat(),
        colorOutHandler_.getType()); // surface albedo
    m_GBuffer->createColorAttachment(normalsOutHandler_.getInternalFormat(), normalsOutHandler_.getFormat(),
        normalsOutHandler_.getType()); // normals
    if (this->chainRenderSlot.CallAs<mmstd_gl::CallRender3DGL>() != nullptr)
        this->chainRenderSlot.CallAs<mmstd_gl::CallRender3DGL>()->SetFramebuffer(m_GBuffer);
    return true;
}
