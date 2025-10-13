package com.zzq.demo6AnimalPerform;

public class Bird extends Animal implements WarmBlood{
    public Bird(String name) {
        super(name);
    }
    public void showFeathers(){
        System.out.println(name + "展开五彩斑斓的羽毛，闪闪发光！");
    }

    @Override
    protected void makeSound() {
        System.out.println(name + "发出叽叽喳喳的声音");
    }

    @Override
    public void perform() {
        System.out.println("🐦类选手" + name + "开始表演");
        jumpAndRoll();
        showFeathers();
        call();
        System.out.println(name + "表演完毕");

    }
}
