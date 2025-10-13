package com.zzq.demo8CallBack;

public class StudeyBa {
    // ================= 2. 定义“学霸同学” (执行耗时任务的类) =================

    /**
     * 这个类扮演“学霸”的角色。
     * 他的工作就是解决难题，这需要一些时间。
     */
    public void solveHardProblem(onTaskCompleted callback) {
        System.out.println("【学霸】这道题有点难度，我要开始计算了");
// 为了模拟耗时计算，我们让程序“睡眠”3秒。
        // 为了不“卡住”你（主程序），学霸在一个“分身”（新线程）里进行计算。
        new Thread(() -> {
            try {
                // 模拟花了3秒钟才算出答案
                Thread.sleep(3000);
                String result = "答案是42";
                System.out.println("【学霸】计算完成结果是" + result);

                // 回调给主程序
                System.out.println("【学霸】蠢蛋让我来告诉你答案");
                callback.onTaskCompleted(result);

            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }).start();
    }
}