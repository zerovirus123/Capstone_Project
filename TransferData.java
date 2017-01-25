package com.pcr_23.pcr_thermal_cycler;

import android.app.ActionBar;
import android.app.Activity;
import android.os.Bundle;
import android.widget.Toast;
import java.io.IOException;
import android.widget.EditText;
import android.content.Intent;
import java.util.HashMap;
import android.support.v7.app.ActionBarActivity;

/**
 * Created by briansia1 on 1/19/16.
 */
public class TransferData extends ActionBarActivity {

    protected void onCreate(Bundle savedInstanceState){

        Intent intent = getIntent();
        HashMap<String, Integer> input_map = (HashMap<String, Integer>) intent.getSerializableExtra("input_map");

        super.onCreate(savedInstanceState);
        setContentView(R.layout.data_transfer);
        BTinit(input_map);
    }

    protected void BTinit(HashMap<String, Integer>input_map){

        Bluetooth BT = new Bluetooth(getApplicationContext(), this);
        try{
            BT.findBT();
            BT.openBT();
        }
        catch(IOException ex){
            Toast toast = Toast.makeText(getApplicationContext(), "Bluetooth Initialization Error.", Toast.LENGTH_LONG);
            toast.show();
        }
        sendInputData(input_map, BT);
        BT.beginListenForData();
    }

    public void sendInputData(HashMap<String, Integer> input_map, Bluetooth BT) {
        //loop through hash map and send both key and value
        for (HashMap.Entry<String, Integer> entry : input_map.entrySet()) {
            String key = entry.getKey();
            int value = entry.getValue();
            byte[] key_byte = key.getBytes();

            //send the key bytes
            for (int index = 0; index < key_byte.length; index++) {
                try {
                    BT.sendData(key_byte[index]);
                    //sendByte(value, BT);
                } catch (IOException ex) {
                    Toast toast = Toast.makeText(getApplicationContext(), "Data send error.", Toast.LENGTH_LONG);
                    toast.show();
                }
            }
        }
    }

    public void sendByte(int value, Bluetooth BT){
        byte[] result = new byte[4];

        result[0] = (byte) (value >> 24);
        result[1] = (byte) (value >> 16);
        result[2] = (byte) (value >> 8);
        result[3] = (byte) (value /*>> 0*/);

        for (int i = 0; i<4; i++){
            try{BT.sendData(result[i]);}
            catch(IOException ex){}
        }
    }

    public void receiveData(byte[] encodedByte){
        

    }

}
