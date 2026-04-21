#include "Characters.h"

XiongDa::XiongDa()
{
    max_hp = 150;
    hp = 150;
    speed = 3;
    anim_left = new Animation(atlas_xiongda_left, 100);
    anim_right = new Animation(atlas_xiongda_right, 100);

    // 熊大的专属遗言
    dead_text = _T("光头强，你给我等着！");

    // 【新增】：熊大力气大，伤害较高
    attack_damage = 2;
}

XiongEr::XiongEr()
{
    max_hp = 100;
    hp = 100;
    speed = 4;
    anim_left = new Animation(atlas_xionger_left, 80);
    anim_right = new Animation(atlas_xionger_right, 80);

    // 熊二的专属遗言
    dead_text = _T("俺 的 蜂 蜜 ！！");

    attack_damage = 1;
}

GuangtouQiang::GuangtouQiang()
{
    max_hp = 70;
    hp = 70;
    speed = 3;
    anim_left = new Animation(atlas_qiang_left, 100);
    anim_right = new Animation(atlas_qiang_right, 100);

    // 光头强的专属遗言
    dead_text = _T("李老板，我的工资啊...");

    // 光头强模型偏窄且步伐较大，将阴影偏移量从默认的15加大到30
    shadow_offset_value = 30;

    attack_damage = 3;
}