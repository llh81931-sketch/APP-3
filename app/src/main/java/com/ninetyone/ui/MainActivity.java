package com.ninetyone.ui;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Button;
import android.widget.LinearLayout;
import android.view.Gravity;
import android.graphics.Color;
import android.widget.Toast;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends Activity {

    private static final String EXEC_NAME = "imgui_chain_1_44.sh";
    private TextView statusText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setGravity(Gravity.CENTER);
        layout.setPadding(50, 50, 50, 50);
        layout.setBackgroundColor(Color.BLACK);

        statusText = new TextView(this);
        statusText.setText("91UI · 安装器");
        statusText.setTextSize(20);
        statusText.setTextColor(Color.WHITE);
        statusText.setGravity(Gravity.CENTER);
        layout.addView(statusText);

        Button installBtn = new Button(this);
        installBtn.setText("安装到 /data/local/tmp");
        installBtn.setTextSize(16);
        installBtn.setOnClickListener(v -> installExecutable());
        layout.addView(installBtn);

        Button runBtn = new Button(this);
        runBtn.setText("启动 91UI (需要 Root)");
        runBtn.setTextSize(16);
        runBtn.setOnClickListener(v -> runExecutable());
        layout.addView(runBtn);

        setContentView(layout);
    }

    private void installExecutable() {
        try {
            File destDir = new File("/data/local/tmp");
            if (!destDir.exists()) {
                destDir.mkdirs();
            }

            File destFile = new File(destDir, EXEC_NAME);

            // 从 APK 的 assets 或 lib 目录复制
            String abi = android.os.Build.SUPPORTED_ABIS[0];
            String srcPath = "lib/" + abi + "/" + EXEC_NAME;

            // 尝试从 native lib 目录加载
            File nativeLib = new File(getApplicationInfo().nativeLibDir, EXEC_NAME);
            if (!nativeLib.exists()) {
                // 备用：从 assets 复制
                try (InputStream in = getAssets().open(EXEC_NAME)) {
                    try (OutputStream out = new FileOutputStream(destFile)) {
                        byte[] buf = new byte[8192];
                        int len;
                        while ((len = in.read(buf)) > 0) {
                            out.write(buf, 0, len);
                        }
                    }
                }
            } else {
                try (InputStream in = new java.io.FileInputStream(nativeLib)) {
                    try (OutputStream out = new FileOutputStream(destFile)) {
                        byte[] buf = new byte[8192];
                        int len;
                        while ((len = in.read(buf)) > 0) {
                            out.write(buf, 0, len);
                        }
                    }
                }
            }

            destFile.setExecutable(true);
            statusText.setText("安装完成: " + destFile.getAbsolutePath());
            Toast.makeText(this, "安装成功", Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            statusText.setText("安装失败: " + e.getMessage());
            Toast.makeText(this, "安装失败: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    private void runExecutable() {
        try {
            Process p = Runtime.getRuntime().exec(new String[]{
                "su", "-c",
                "cd /data/local/tmp && ./" + EXEC_NAME
            });
            statusText.setText("91UI 已启动");
            Toast.makeText(this, "已通过 root 启动", Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            statusText.setText("启动失败: " + e.getMessage());
            Toast.makeText(this, "启动失败，请确认已 Root", Toast.LENGTH_LONG).show();
        }
    }
}
