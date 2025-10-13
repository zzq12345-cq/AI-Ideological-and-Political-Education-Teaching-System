package com.zzq.demo6AnimalPerform;

// 标记接口，用来判断动物是否是恒温动物
public interface WarmBlood {
    default boolean isWarmBlood(){
        return true;
    }
}
