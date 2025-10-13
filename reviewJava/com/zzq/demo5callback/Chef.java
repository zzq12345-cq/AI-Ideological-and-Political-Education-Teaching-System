package com.zzq.demo5callback;

public class Chef {
    public void cook(Callback callback) {
        System.out.println("厨师:开始做饭");
        System.out.println("厨师：饭做好了");

        callback.complete();
    }
}
