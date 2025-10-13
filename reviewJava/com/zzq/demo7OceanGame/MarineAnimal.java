package com.zzq.demo7OceanGame;

public abstract class MarineAnimal {
    protected String name;

    // 传入海洋动物的名字
    public  MarineAnimal(String name){
        this.name = name;
    }

    // 所有动物都可以入水潜游
    public void diveAndSwim(){
        System.out.println(name + "入水潜游，溅起巨大浪花！");
    }

    // 所有动物都可以发出声音
    public void call(){
        deepBreath();
        makeSound();
    }
    private void deepBreath(){
        System.out.println(name + "吸气");
    }

   // 各种动物的叫声不同
   protected abstract void makeSound();

    // 表演流程
    public abstract void perform();
}
