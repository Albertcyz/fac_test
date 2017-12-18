#!/bin/sh

/home/root/key_rgb &                                                                
sleep 1                                                                             
/home/root/mp3_player &                                                             
                                                                                    
                             
echo "===================================="
echo "========== start gw================="
echo "===================================="
/lumi/app/miio/start_ot.sh &               
/home/root/gw &                            
sleep 3                                    
/home/root/wifi_capture &  
