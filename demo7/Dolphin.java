package com.zzq.demo7OceanGame;

public class Dolphin extends MarineAnimal implements AirBreather {
    public Dolphin (String name){
        super(name);
    }

    public void jet(){
        System.out.println(name + "喷出海浪");
    }
    @Override
    protected void makeSound() {
        System.out.println("发出了呜呜呜的鸣声");
    }

    @Override
    public void perform() {
        System.out.println(name + "开始表演");
        diveAndSwim();
        jet();
        call();
        System.out.println(name + "的表演结束，掌声雷动！");
    }
}
