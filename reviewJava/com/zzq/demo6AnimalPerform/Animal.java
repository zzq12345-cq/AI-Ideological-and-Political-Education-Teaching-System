package com.zzq.demo6AnimalPerform;

public abstract class Animal {
    protected String name;

    // 传入动物名字
    public Animal(String name){
        this.name = name;
    }

    // 所有动物都可以跳跃翻滚
    public void jumpAndRoll(){
        System.out.println(name + "跳起来在空中翻了个漂亮的跟斗！");
    }

    // 发出叫声 System.out.println("🎉 比赛圆满结束，感谢所有动物的精彩参与！");: 张嘴 + 发声
    public void call(){
        openMouth();
        makeSound();
    }
    private void openMouth(){
        System.out.println(name + "张嘴");
    }

    // 各种动物的叫声不同
     protected abstract void makeSound();

    // 表演流程
    public abstract void perform();
}
