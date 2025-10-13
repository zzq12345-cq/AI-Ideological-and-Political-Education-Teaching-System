package com.zzq.demo5callback;

public class zzq {
    public static void main(String[] args) {
        Cq cq = new Cq();
        System.out.println("cq:zzq开始做饭");
        cq.cook(new Callback1() {
            @Override
            public void complete() {
                System.out.println("cq:准备开饭");
            }
        });
    }
}
