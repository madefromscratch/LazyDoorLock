package com.example.nfcutils;

import android.content.Context;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import org.w3c.dom.Text;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

public class MainActivity extends AppCompatActivity {

    Switch openCloseSwitch;
    TextView infoTextView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        infoTextView = findViewById(R.id.InfoTextView);
        openCloseSwitch = findViewById(R.id.OpenCloseSwitch);
        openCloseSwitch.setChecked(readCheckedState(getApplicationContext()).equals("true"));
        openCloseSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                writeCheckedState(isChecked, getApplicationContext());
            }
        });
        if (getIntent().getStringExtra("PerformedAction") != null) {
            infoTextView.setText("LOCK " + getIntent().getStringExtra("PerformedAction") + "!");
        }
    }

    private void writeCheckedState(boolean state, Context context) {
        try {
            OutputStreamWriter outputStreamWriter = new OutputStreamWriter(context.openFileOutput(getString(R.string.OpenCloseStateFile), Context.MODE_PRIVATE));
            outputStreamWriter.write(String.valueOf(state));
            outputStreamWriter.close();
        }
        catch (IOException e) {
            Log.e("Exception", "File write failed: " + e.toString());
            e.printStackTrace();
        }
    }


    private String readCheckedState(Context context) {

        String ret = "true";

        try {
            InputStream inputStream = context.openFileInput(getString(R.string.OpenCloseStateFile));

            if ( inputStream != null ) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
                String receiveString = "";
                StringBuilder stringBuilder = new StringBuilder();

                while ( (receiveString = bufferedReader.readLine()) != null ) {
                    stringBuilder.append(receiveString);
                }

                inputStream.close();
                ret = stringBuilder.toString();
            }
        }
        catch (FileNotFoundException e) {
            Log.e("login activity", "File not found: " + e.toString());
            e.printStackTrace();
        } catch (IOException e) {
            Log.e("login activity", "Can not read file: " + e.toString());
            e.printStackTrace();
        }

        return ret;
    }
}
