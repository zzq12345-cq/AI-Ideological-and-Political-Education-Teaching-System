package com.zzq.demo6AnimalPerform;

public class Mammal extends  Animal implements WarmBlood{
    public Mammal(String name) {
        super(name);
    }

    @Override
    protected void makeSound() {
        System.out.println(name + "发出强而有力的声音！");
    }

    @Override
    public void perform() {
        System.out.println("哺乳类选手" + name + "开始表演");
        jumpAndRoll();
        capture();
        call();
        System.out.println(name + "表演完毕");
    }

    public void capture(){
        System.out.println(name + "迅速扑向猎物，展示了强壮的捕食本领！");
    }


}
