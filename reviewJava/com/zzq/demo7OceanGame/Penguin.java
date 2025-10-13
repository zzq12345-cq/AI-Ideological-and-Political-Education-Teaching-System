package com.zzq.demo7OceanGame;

public class Penguin extends MarineAnimal implements AirBreather {
    public Penguin(String name){
        super(name);
    }

    // “拍翅游泳表演”
    public void swimInAir(){
        System.out.println(name + "开始拍翅游泳表演");
    }
    @Override
    protected void makeSound() {
        System.out.println("发出了可爱的叫声");
    }

    @Override
    public void perform() {
        System.out.println(name + "开始表演");
        diveAndSwim();
        swimInAir();
        call();
        System.out.println(name + "的表演结束，掌声雷动！");
    }

}
