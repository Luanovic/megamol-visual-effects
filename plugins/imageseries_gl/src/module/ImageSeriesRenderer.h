#ifndef IMAGESERIES_GL_SRC_MODULE_IMAGESERIESRENDERER_HPP_
#define IMAGESERIES_GL_SRC_MODULE_IMAGESERIESRENDERER_HPP_

#include "ImageDisplay2D.h"
#include "imageseries/ImageSeries2DCall.h"

#include "mmcore/CallerSlot.h"
#include "mmcore/param/ParamSlot.h"
#include "mmcore/utility/graphics/AbstractBitmapCodec.h"
#include "mmcore_gl/view/Renderer2DModuleGL.h"
#include "vislib/Pair.h"
#include "vislib/RawStorage.h"
#include "vislib/SmartPtr.h"
#include "vislib/math/Rectangle.h"
#include "vislib_gl/graphics/gl/GLSLShader.h"
#include "vislib_gl/graphics/gl/OpenGLTexture2D.h"

#include <memory>

namespace megamol::ImageSeries::GL {

class ImageSeriesRenderer : public core_gl::view::Renderer2DModuleGL {
public:
    /**
     * Answer the name of this module.
     *
     * @return The name of this module.
     */
    static const char* ClassName() {
        return "ImageSeriesRenderer";
    }

    /**
     * Answer a human readable description of this module.
     *
     * @return A human readable description of this module.
     */
    static const char* Description() {
        return "Displays images from an image series.";
    }

    /**
     * Answers whether this module is available on the current system.
     *
     * @return 'true' if the module is available, 'false' otherwise.
     */
    static bool IsAvailable() {
        return true;
    }

    /** Ctor. */
    ImageSeriesRenderer();

    /** Dtor. */
    virtual ~ImageSeriesRenderer() override;

protected:
    /**
     * Implementation of 'Create'.
     *
     * @return 'true' on success, 'false' otherwise.
     */
    bool create() override;

    /**
     * The get extents callback. The module should set the members of
     * 'call' to tell the caller the extents of its data (bounding boxes
     * and times).
     *
     * @param call The calling call.
     *
     * @return The return value of the function.
     */
    bool GetExtents(core_gl::view::CallRender2DGL& call) override;

    /**
     * Implementation of 'Release'.
     */
    void release() override;

    /**
     * The render callback.
     *
     * @param call The calling call.
     *
     * @return The return value of the function.
     */
    bool Render(core_gl::view::CallRender2DGL& call) override;

private:
    std::unique_ptr<ImageDisplay2D> display;
    std::shared_ptr<const AsyncImageData2D> currentImage;

    core::CallerSlot getDataCaller;

    ImageSeries2DCall::Output metadata;
};

} // namespace megamol::ImageSeries::GL

#endif
