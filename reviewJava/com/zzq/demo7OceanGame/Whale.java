package com.zzq.demo7OceanGame;

public class Whale extends MarineAnimal implements AirBreather{
    public Whale(String name){
        super(name);
    }

    public void jet(){
        System.out.println( name + " 喷射出海浪！");
    }
    @Override
    protected void makeSound() {
        System.out.println("唱起了深沉而悠长的鲸歌！");
    }

    @Override
    public void perform() {
        System.out.println("哺乳类选手" + name + "开始表演");
        diveAndSwim();
        jet();
        call();
        System.out.println(name + "的表演结束，掌声雷动！");

    }
}
