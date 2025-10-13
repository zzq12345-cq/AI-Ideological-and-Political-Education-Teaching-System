package com.zzq.demo7OceanGame;

public class Main {
    public static void main(String[] args) {
        System.out.println("\uD83C\uDF0A 欢迎来到海洋才艺大赛！");
        MarineAnimal dolphin = new Dolphin("海豚");
        MarineAnimal shark = new Shark("鲨鱼");
        MarineAnimal whale = new Whale("鲸鱼");
        MarineAnimal penguin = new Penguin("企鹅");

        dolphin.perform();
        System.out.println("========================");
        shark.perform();
        System.out.println("========================");
        whale.perform();
        System.out.println("========================");
        penguin.perform();

        System.out.println("🎉 比赛圆满结束，感谢所有动物的精彩参与！");
    }
}
