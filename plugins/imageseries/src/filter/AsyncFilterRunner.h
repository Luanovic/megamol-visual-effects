#ifndef SRC_IMAGESERIES_FILTER_ASYNCFILTERRUNNER_HPP_
#define SRC_IMAGESERIES_FILTER_ASYNCFILTERRUNNER_HPP_

#include "imageseries/AsyncImageData2D.h"

#include <functional>
#include <memory>

namespace megamol::ImageSeries::filter {

/**
 * Encapsulates image series processing functionality with support for asynchronous execution.
 *
 * Inputs and outputs are provided via async image objects.
 * The filter itself may also perform its work on a separate thread.
 *
 * TODO: not actually multi-threaded yet!
 */
class AsyncFilterRunner {
public:
    using AsyncImageData = std::shared_ptr<const AsyncImageData2D>;
    using ImageData = std::shared_ptr<const AsyncImageData2D::BitmapImage>;

    AsyncFilterRunner();
    ~AsyncFilterRunner();

    template<typename Filter, typename... Args>
    AsyncImageData run(Args&&... args) {
        std::shared_ptr<Filter> filter = std::make_shared<Filter>(std::forward<Args>(args)...);
        return runFunction([filter]() { return (*filter)(); }, filter->getByteSize());
    }

    AsyncImageData runFunction(std::function<ImageData()> filter, std::size_t byteSize);
};

} // namespace megamol::ImageSeries::filter


#endif
