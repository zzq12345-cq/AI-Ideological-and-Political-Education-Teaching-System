package com.zzq.demo9coffee;

public class MakeCoffee {
    // ================= 1. 定义“咖啡” (被执行耗时任务的类) =================
    /**
     * 接收订单并开始制作咖啡（异步）。
     * @param customerName 顾客姓名
     * @param coffeeType   咖啡类型
     * @param callback     咖啡做好后用于通知的回调
     */
    public void makeCoffee(String customerName, String coffeeType,    CoffeeCallback callback){
        System.out.println("【杰伦】好的，" + customerName + "，您点的 " +coffeeType+"  请稍等");

        // 模拟咖啡制作耗时
        new Thread(() -> {
            // 模拟咖啡制作耗时
            try {
                System.out.println("【杰伦】正在制作您的咖啡");
                Thread.sleep(5000);

                String finishcoffee = "一杯美味的" + coffeeType;
                System.out.println("【杰伦】" + customerName +  "，" + finishcoffee + " 制作完成");

                // 回调给主程序
                callback.onCoffeeReady(finishcoffee);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

        }).start();
    }
}
