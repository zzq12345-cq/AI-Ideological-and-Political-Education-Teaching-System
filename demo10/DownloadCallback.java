package com.zzq.demo10URL;

public interface DownloadCallback {
    // 包含成功与失败
    void onSuccess(String imageData);
    void onFailure(String errorMessage);
}
