package com.zzq.demo9coffee;

public class Main {
    public static void main(String[] args) {
        System.out.println("星巴克开门了，进去点单");
        MakeCoffee mk = new MakeCoffee();

        // 创建顾客张三
        String 张三 = "张三";
        mk.makeCoffee(张三, "拿铁", new CoffeeCallback() {
            @Override
            public void onCoffeeReady(String coffee) {
                System.out.println("【" + 张三 + "】：听到喊我了，我的 " + coffee + " 好了！");
            }
        });

        // 创建顾客李四
        String 李四 = "李四";
        mk.makeCoffee(李四, "美式", new CoffeeCallback() {
            @Override
            public void onCoffeeReady(String coffee) {
                System.out.println("【" + 李四 + "】：听到喊我了，我的 " + coffee + " 好了！");
            }
        });

        System.out.println("\n【张三和李四】：我们点好单了，找个座位坐下玩手机。\n");
            }
        }
