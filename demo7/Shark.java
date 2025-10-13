package com.zzq.demo7OceanGame;

public class Shark extends MarineAnimal {
    public Shark(String name) {
        super(name);
    }
    @Override
    protected void makeSound() {
        System.out.println(name + "刚想发出声音");
    }

    @Override
    public void perform() {
        call();
        System.out.println("被告知没有参赛资格呜呜呜");
    }

}