package com.zzq.demo6AnimalPerform;

import java.util.ArrayList;
import java.util.List;

public class Main {
    public static void main(String[] args) {
        System.out.println("\uD83C\uDF33 森林年度才艺大赛拉开帷幕！\uD83C\uDF33\n");
        Animal bird = new Bird("孔雀");
        Animal mammal = new Mammal("老虎");
        Animal reptile = new Others("蜥蜴");

        bird.perform();
        mammal.perform();
        reptile.perform();

        System.out.println("🎉 比赛圆满结束，感谢所有动物的精彩参与！");
    }
}
