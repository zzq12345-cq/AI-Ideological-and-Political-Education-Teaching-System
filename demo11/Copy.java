package com.zzq.demo11Shuzu;

import java.util.Arrays;

public class Copy {
    public static void main(String[] args) {
        // 数组复制
        int [] a = {1,2,3,4,5,6,7,8,9,10};
        int n = a.length;
        int [] b = new int[n];
        for (int i = 0; i < n; i++) {
            b[i] = a[i];
        }
        System.out.println("数组b = " + Arrays.toString(b));
    }
}
