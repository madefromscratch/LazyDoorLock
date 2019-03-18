package com.example.nfcutils;

import android.content.Context;
import android.content.Intent;
import android.nfc.cardemulation.HostApduService;
import android.os.Bundle;
import android.util.Log;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

public class MyHostApduService extends HostApduService {

    boolean open = true;

    @Override
    public byte[] processCommandApdu(byte[] apdu, Bundle extras) {
        // if got hello
        // pull open/close from app
        // send
        // set timer: cannot respond again until timer expires.
        String apduString = "";
        try{
            apduString = new String(apdu, "UTF-8");
        } catch(Exception e) {
            Log.d("apdu debug", "to string failed");
            e.printStackTrace();
        }

        byte[] returnCode = {0x48, 0x45, 0x4C, 0x4C, 0x4F}; // HELLO
        if (apduString.equals("QUERY")) {
            if ( readCheckedState(getApplicationContext()).equals("true")) {
                returnCode = new byte[]{0x4F, 0x50, 0x45, 0x4E}; // OPEN
                writeCheckedState(false,getApplicationContext());
            } else {
                returnCode = new byte[]{0x43, 0x4C, 0X4F, 0X53, 0X45}; // CLOSE
                writeCheckedState(true, getApplicationContext());
            }

        } else if (apduString.equals("OPEN") || apduString.equals("CLOSE")) {
            Intent confirmationIntent = new Intent(this, MainActivity.class);
            confirmationIntent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT | Intent.FLAG_ACTIVITY_CLEAR_TASK);
            confirmationIntent.putExtra("PerformedAction", apduString);
            returnCode = new byte[]{0x41, 0x43, 0x4B}; // ACK
            startActivity(confirmationIntent);
        }
        return returnCode;
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

    @Override
    public void onDeactivated(int id) { }

    public MyHostApduService() { }

}
