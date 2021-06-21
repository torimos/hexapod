﻿using System;
using System.Diagnostics;
using UnityEngine;

public class Hexapod
{
    private Leg[] legs = new Leg[HexConfig.LegsCount];
    private ServoState[] servos = new ServoState[26];
    private GameObject hexapod;
    private Stopwatch last_leg_updated;

    public void Create()
    {
        last_leg_updated = new Stopwatch();
        last_leg_updated.Restart();
        for (int i = 0; i < servos.Length; i++)
            servos[i] = new ServoState();

        Create3DModel();
        Reset();
    }

    public void Update()
    {
        if (last_leg_updated.ElapsedMilliseconds > 20)
        {
            for (int i = 0; i < servos.Length; i++)
            {
                servos[i].Update();
            }
            for (int i = 0; i < HexConfig.LegsCount; i++)
            {
                uint sdC = (uint)servos[HexConfig.ServoMap[i, 0]].position;
                uint sdF = (uint)servos[HexConfig.ServoMap[i, 1]].position;
                uint sdT = (uint)servos[HexConfig.ServoMap[i, 2]].position;

                if (sdC == 0 || sdF == 0 || sdT == 0)
                {
                    legs[i].Reset();
                    return;
                }

                float d = 10f;
                float c = ((int)(sdC & 0xFFFF) - 1500) / d;
                float f = ((int)(sdF & 0xFFFF) - 1500) / d;
                float t = ((int)(sdT & 0xFFFF) - 1500) / d;

                legs[i].Update(c, f, t);
            }
            //UpdateBody();
            last_leg_updated.Restart();
        }
    }

    public void ProcessFrameData(uint[] data)
    {
        for (int i = 0; i < servos.Length; i++)
        {
            servos[i].ProcessData(data[i]);
        }
    }

    public void Reset()
    {
        legs[0].Update(HexConfig.coxaInitAngle, HexConfig.femurInitAngle, HexConfig.tibiaInitAngle);
        legs[1].Update(HexConfig.coxaInitAngle, HexConfig.femurInitAngle, HexConfig.tibiaInitAngle);
        legs[2].Update(HexConfig.coxaInitAngle, HexConfig.femurInitAngle, HexConfig.tibiaInitAngle);

        legs[3].Update(-HexConfig.coxaInitAngle, -HexConfig.femurInitAngle, -HexConfig.tibiaInitAngle);
        legs[4].Update(-HexConfig.coxaInitAngle, -HexConfig.femurInitAngle, -HexConfig.tibiaInitAngle);
        legs[5].Update(-HexConfig.coxaInitAngle, -HexConfig.femurInitAngle, -HexConfig.tibiaInitAngle);
    }

    public void UpdateLeg(uint[] servoData, int legIdx)
    {
        uint sdC = servoData[HexConfig.ServoMap[legIdx, 0]];
        uint sdF = servoData[HexConfig.ServoMap[legIdx, 1]];
        uint sdT = servoData[HexConfig.ServoMap[legIdx, 2]];

        if (sdC == 0 || sdF == 0 || sdT == 0)
        {
            legs[legIdx].Reset();
            return;
        }

        float d = 10f;
        float c = ((int)(sdC & 0xFFFF) - 1500) / d;
        float f = ((int)(sdF & 0xFFFF) - 1500) / d;
        float t = ((int)(sdT & 0xFFFF) - 1500) / d;

        legs[legIdx].Update(c, f, t);

        UpdateBody();
    }

    private void UpdateBody()
    {
        float minLegY = 0;
        for (int i = 0; i < legs.Length; i++)
        {
            minLegY += legs[i].tibiaEnd.transform.position.y;
        }
        minLegY /= 6;

        //minLegY += 0.26f;
        //minLegY -= HexConfig.otherJointSize/2;

        //Debug.Log(minLegY);
        //hexapod.transform.position = new Vector3(0, -minLegY, 0);
    }

    private void Create3DModel()
    {
        hexapod = new GameObject("hexapod");

        var hexaBase = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
        hexaBase.name = "hexaBase";
        hexaBase.transform.parent = hexapod.transform;
        hexaBase.transform.localScale = new Vector3(HexConfig.legsZOffset * 2, HexConfig.bodyHeight / 2, HexConfig.legsZOffset * 2);
        hexaBase.transform.localPosition = new Vector3(0, HexConfig.bodyOffset, 0);
        hexaBase.GetComponent<Renderer>().material.color = Color.gray;

        var hexaHead = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
        hexaHead.name = "hexaHead";
        hexaHead.transform.parent = hexaBase.transform;
        hexaHead.transform.localScale = new Vector3(0.25f, 0.7f, 0.25f);
        hexaHead.transform.localPosition = new Vector3(0, 0, -HexConfig.legsXOffset + 0.1f);
        hexaHead.GetComponent<Renderer>().material.color = Color.black;

        for (int i = 0; i < legs.Length; i++) legs[i] = new Leg();
        legs[0].Create(hexapod, "RR", new Vector3(-HexConfig.legsXOffset, HexConfig.legsOffsetY, HexConfig.legsZOffset), HexConfig.legsAngleOffset, true);
        legs[1].Create(hexapod, "RM", new Vector3(-HexConfig.legsXOffset * 2, HexConfig.legsOffsetY, 0), 0, true);
        legs[2].Create(hexapod, "RF", new Vector3(-HexConfig.legsXOffset, HexConfig.legsOffsetY, -HexConfig.legsZOffset), -HexConfig.legsAngleOffset, true);

        legs[3].Create(hexapod, "LR", new Vector3(HexConfig.legsXOffset, HexConfig.legsOffsetY, HexConfig.legsZOffset), HexConfig.legsAngleOffset);
        legs[4].Create(hexapod, "LM", new Vector3(HexConfig.legsXOffset * 2, HexConfig.legsOffsetY, 0), 0);
        legs[5].Create(hexapod, "LF", new Vector3(HexConfig.legsXOffset, HexConfig.legsOffsetY, -HexConfig.legsZOffset), -HexConfig.legsAngleOffset);
    }
}
