package com.zzq.demo6AnimalPerform;

public class Others extends Animal {
    public Others(String name){
        super(name);
    }

    @Override
    protected void makeSound() {
        System.out.println(name + "刚想发出声音");
    }

    @Override
    public void perform() {
        call();
        System.out.println(name + "被告知没有比赛资格");
    }
}
