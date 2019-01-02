#ifndef SWITCHBUTTONSTYLE_H
#define SWITCHBUTTONSTYLE_H
/**Store the style of the toggle button to reduce the amount of code*/
#include <string>
const std::string checkedLeftBtStyle = "QPushButton{background-color:rgba(59,153,252,100%);padding:0px 10px 0px 10px;\
                                        color: black;   border-top-left-radius: 10px; border-bottom-left-radius: 10px;  border: 2px groove #ffffff; border-style: solid;}\
                                        QPushButton:hover{background-color:rgb(59,153,252); color: black;}\
                                        QPushButton:pressed{background-color:rgb(59,153,252); border-style: inset; }";

 const std::string checkedMiddleBtStyle = "QPushButton{background-color:rgba(59,153,252,100%);padding:0px 10px 0px 10px;\
                                        color: black;border: 2px groove #ffffff; border-style: solid;}\
                                        QPushButton:hover{background-color:rgb(59,153,252); color: black;}\
                                        QPushButton:pressed{background-color:rgb(59,153,252); border-style: inset; }";

const std::string checkedRightBtStyle = "QPushButton{background-color:rgba(59,153,252,100%);padding:0px 10px 0px 10px;\
                                        color: black;   border-top-right-radius: 10px; border-bottom-right-radius: 10px;  border: 2px groove #ffffff; border-style: solid;}\
                                        QPushButton:hover{background-color:rgb(59,153,252); color: black;}\
                                        QPushButton:pressed{background-color:rgb(59,153,252); border-style: inset; }";

const std::string commonLeftBtStyle = "QPushButton{background-color:rgba(220,220,220,100%);padding:0px 10px 0px 10px;\
                                        color: black;   border-top-left-radius: 10px; border-bottom-left-radius: 10px; border: 2px groove #ffffff; border-style: solid;}\
                                        QPushButton:hover{background-color:rgb(59,153,252); color: black;}\
                                        QPushButton:pressed{background-color:rgb(59,153,252); border-style: inset; }";

const std::string commonMiddleBtStyle = "QPushButton{background-color:rgba(220,220,220,100%);padding:0px 10px 0px 10px;\
                                        color: black;border: 2px groove #ffffff; border-style: solid;}\
                                        QPushButton:hover{background-color:rgb(59,153,252); color: black;}\
                                        QPushButton:pressed{background-color:rgb(59,153,252); border-style: inset; }";

const std::string commonRightBtStyle = "QPushButton{background-color:rgba(220,220,220,100%);padding:0px 10px 0px 10px;\
                                        color: black;   border-top-right-radius: 10px; border-bottom-right-radius: 10px;  border: 2px groove #ffffff; border-style: solid;}\
                                        QPushButton:hover{background-color:rgb(59,153,252); color: black;}\
                                        QPushButton:pressed{background-color:rgb(59,153,252); border-style: inset; }";
#endif //SWITCHBUTTONSTYLE_H