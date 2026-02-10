#pragma once

#include "AnimationController.hpp"

class MenuController
{
public:
    void render(AnimationController &animCtrl, ArterialTree &tree, TreeRenderer &renderer, bool hideMainPanel = false);
};