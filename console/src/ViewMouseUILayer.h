/*
 * ViewMouseUILayer.h
 *
 * Copyright (C) 2016 MegaMol Team
 * Alle Rechte vorbehalten. All rights reserved.
 */
#pragma once

#include "AbstractUILayer.h"

namespace megamol {
namespace console {

    /**
     * This UI layer propagates mouse events to the connected (core) view
     */
    class ViewMouseUILayer : public AbstractUILayer {
    public:
        ViewMouseUILayer(gl::Window& wnd, void * viewHandle);
        virtual ~ViewMouseUILayer();

        virtual void OnResize(int w, int h);

		virtual bool OnKey(core::view::Key key, core::view::KeyAction action, core::view::Modifiers mods);
        virtual bool OnChar(unsigned int codePoint);
        virtual bool OnMouseMove(double x, double y);
        virtual bool OnMouseButton(core::view::MouseButton button, core::view::MouseButtonAction action, core::view::Modifiers mods);
        virtual bool OnMouseScroll(double x, double y);

    private:
        void *hView; // handle memory is owned by Window
    };

}
}
