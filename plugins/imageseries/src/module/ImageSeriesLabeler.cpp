#include "ImageSeriesLabeler.h"
#include "imageseries/ImageSeries2DCall.h"

#include "mmcore/misc/PngBitmapCodec.h"
#include "mmcore/param/BoolParam.h"
#include "mmcore/param/EnumParam.h"
#include "mmcore/param/FloatParam.h"
#include "mmcore/param/IntParam.h"
#include "mmcore/param/StringParam.h"

#include "../filter/BlobLabelFilter.h"
#include "../util/ImageUtils.h"

using Log = megamol::core::utility::log::Log;
using Presentation = megamol::core::param::AbstractParamPresentation::Presentation;

namespace megamol::ImageSeries {

ImageSeriesLabeler::ImageSeriesLabeler()
        : getDataCallee("getData", "Returns data from the image series for the requested timestamp.")
        , getInputCaller("requestInputImageSeries", "Requests image data from a series.")
        , getMaskCaller("requestMaskImageSeries", "Requests mask data from an image series.")
        , maskFrameParam("Mask frame", "Image timestamp to use for applying a mask.")
        , minBlobSizeParam("Min blob size", "Blobs with fewer pixels than this threshold are discarded.")
        , thresholdParam("Value threshold", "Per-pixel value threshold for labeling.")
        , negateThresholdParam("Negate value threshold", "Negates the value threshold for labeling.")
        , imageCache([](const AsyncImageData2D& imageData) { return imageData.getByteSize(); }) {

    getInputCaller.SetCompatibleCall<typename ImageSeries::ImageSeries2DCall::CallDescription>();
    MakeSlotAvailable(&getInputCaller);

    getMaskCaller.SetCompatibleCall<typename ImageSeries::ImageSeries2DCall::CallDescription>();
    MakeSlotAvailable(&getMaskCaller);

    getDataCallee.SetCallback(ImageSeries2DCall::ClassName(),
        ImageSeries2DCall::FunctionName(ImageSeries2DCall::CallGetData), &ImageSeriesLabeler::getDataCallback);
    getDataCallee.SetCallback(ImageSeries2DCall::ClassName(),
        ImageSeries2DCall::FunctionName(ImageSeries2DCall::CallGetMetaData), &ImageSeriesLabeler::getMetaDataCallback);
    MakeSlotAvailable(&getDataCallee);

    maskFrameParam << new core::param::FloatParam(0.f, 0.f, 1.f);
    maskFrameParam.Parameter()->SetGUIPresentation(Presentation::Slider);
    maskFrameParam.SetUpdateCallback(&ImageSeriesLabeler::filterParametersChangedCallback);
    MakeSlotAvailable(&maskFrameParam);

    minBlobSizeParam << new core::param::IntParam(10, 0, 100);
    minBlobSizeParam.Parameter()->SetGUIPresentation(Presentation::Slider);
    minBlobSizeParam.SetUpdateCallback(&ImageSeriesLabeler::filterParametersChangedCallback);
    MakeSlotAvailable(&minBlobSizeParam);

    thresholdParam << new core::param::FloatParam(0.5f, 0.f, 1.f);
    thresholdParam.Parameter()->SetGUIPresentation(Presentation::Slider);
    thresholdParam.SetUpdateCallback(&ImageSeriesLabeler::filterParametersChangedCallback);
    MakeSlotAvailable(&thresholdParam);

    negateThresholdParam << new core::param::BoolParam(0);
    negateThresholdParam.Parameter()->SetGUIPresentation(Presentation::Checkbox);
    negateThresholdParam.SetUpdateCallback(&ImageSeriesLabeler::filterParametersChangedCallback);
    MakeSlotAvailable(&negateThresholdParam);

    // Set default image cache size to 512 MB
    imageCache.setMaximumSize(512 * 1024 * 1024);
}

ImageSeriesLabeler::~ImageSeriesLabeler() {
    Release();
}

bool ImageSeriesLabeler::create() {
    filterRunner = std::make_unique<filter::AsyncFilterRunner>();
    return true;
}

void ImageSeriesLabeler::release() {
    filterRunner = nullptr;
    imageCache.clear();
}

bool ImageSeriesLabeler::getDataCallback(core::Call& caller) {
    if (auto* call = dynamic_cast<ImageSeries2DCall*>(&caller)) {
        // Try to get mask
        ImageSeries2DCall::Output mask;
        if (auto* getMask = getMaskCaller.CallAs<ImageSeries::ImageSeries2DCall>()) {
            ImageSeries2DCall::Input maskInput;
            maskInput.time = maskFrameParam.Param<core::param::FloatParam>()->Value();
            getMask->SetInput(maskInput);
            if ((*getMask)(ImageSeries::ImageSeries2DCall::CallGetData)) {
                mask = getMask->GetOutput();
            }
        }

        if (auto* getInput = getInputCaller.CallAs<ImageSeries::ImageSeries2DCall>()) {
            // Pass through input parameters
            getInput->SetInput(call->GetInput());
            if ((*getInput)(ImageSeries::ImageSeries2DCall::CallGetData)) {
                auto output = getInput->GetOutput();

                // Retrieve cached image or run filter on input data
                auto hash = util::combineHash(output.getHash(), mask.getHash());
                output.imageData = imageCache.findOrCreate(hash, [=](AsyncImageData2D::Hash) {
                    filter::BlobLabelFilter::Input filterInput;
                    filterInput.image = output.imageData;
                    filterInput.mask = mask.imageData;
                    filterInput.minBlobSize = minBlobSizeParam.Param<core::param::IntParam>()->Value();
                    filterInput.threshold = thresholdParam.Param<core::param::FloatParam>()->Value();
                    filterInput.negateThreshold = negateThresholdParam.Param<core::param::BoolParam>()->Value();
                    return filterRunner->run<filter::BlobLabelFilter>(filterInput);
                });

                call->SetOutput(output);
                return true;
            }
        }
    }
    return false;
}

bool ImageSeriesLabeler::getMetaDataCallback(core::Call& caller) {
    if (auto* call = dynamic_cast<ImageSeries2DCall*>(&caller)) {
        if (auto* getInput = getInputCaller.CallAs<ImageSeries::ImageSeries2DCall>()) {
            if ((*getInput)(ImageSeries::ImageSeries2DCall::CallGetMetaData)) {
                // Pass through metadata
                call->SetOutput(getInput->GetOutput());
                return true;
            }
        }
    }
    return false;
}

bool ImageSeriesLabeler::filterParametersChangedCallback(core::param::ParamSlot& param) {
    imageCache.clear();
    return true;
}


} // namespace megamol::ImageSeries
