package com.zzq.demo5callback;

public  class Cq {
    public void cook(Callback1 ck){
        System.out.println("zzq:开始做饭");
        System.out.println("zzq:饭做好了");
        ck.complete();
    }
}
