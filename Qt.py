import sys
from PyQt5.Qt import *
from PyQt5.Qt import QPixmap, QThread
import socket
global ip_addr
global result_info
ip_addr = ''
result_info = ''


class ScannerWindow(QWidget):

    def __init__(self):
        super().__init__()
        self.scanresult = ''
        # 初始化控件
        self.image_label = QLabel()
        self.file_path = QLabel("IP 地址:")
        self.resultname = QLabel("扫描结果：")
        self.file_path_edit = QLineEdit()
        self.result = QTextEdit()
        self.start = QLabel("开始端口号：")
        self.start_edit = QLineEdit()
        self.end = QLabel("结束端口号：")
        self.end_edit = QLineEdit()

        self.okbutton1 = QPushButton("扫描")
        self.okbutton2 = QPushButton("停止")
        self.cancelbutton = QPushButton("取消")

        self.okbutton1.clicked.connect(okbuttonclicked_1)
        self.okbutton2.clicked.connect(self.okbuttonclicked_2)
        self.cancelbutton.clicked.connect(self.cancelbuttonclicked)

        # 控件样式设置
        self.image = QPixmap("image.jpg")
        self.image_label.setPixmap(self.image)
        # 初始化界面布局
        self.initUI()

    # 初始化UI界面并设置主题窗口的属性
    def initUI(self):
        self.setWindowTitle("Scanner based on TCP")
        self.center()
        self.Layout()
        # self.show()

    def closeEvent(self, closeEvent):
        reply = QMessageBox.question(self,
                                     "Scanner based on TCP",
                                     "确定关闭吗？",
                                     QMessageBox.Yes | QMessageBox.No,
                                     QMessageBox.Yes)
        if reply == QMessageBox.Yes:
            closeEvent.accept()  # 如果点击yes按钮则表示接受此事件
        else:
            closeEvent.ignore()  # 如果点击no按钮则表示忽略此事件

    # 控制窗口显示在屏幕中心的方法
    def center(self):

        # 获得窗口
        qr = self.frameGeometry()
        # 获得屏幕中心点
        cp = QDesktopWidget().availableGeometry().center()
        # 显示到屏幕中心
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    def Layout(self):
        hbox1 = QHBoxLayout()
        hbox1.addWidget(self.file_path)
        hbox1.addWidget(self.file_path_edit)

        hbox3 = QHBoxLayout()
        hbox3.addWidget(self.start)
        hbox3.addWidget(self.start_edit)

        hbox4 = QHBoxLayout()
        hbox4.addWidget(self.end)
        hbox4.addWidget(self.end_edit)

        hbox2 = QHBoxLayout()
        hbox2.addStretch(1)
        hbox2.addWidget(self.cancelbutton)
        hbox2.addWidget(self.okbutton2)
        hbox2.addWidget(self.okbutton1)

        vbox = QVBoxLayout()
        vbox.addWidget(self.image_label)
        vbox.addLayout(hbox1)
        vbox.addLayout(hbox3)
        vbox.addLayout(hbox4)
        vbox.addWidget(self.resultname)
        vbox.addWidget(self.result)
        vbox.addLayout(hbox2)
        self.setLayout(vbox)
        self.setGeometry(600, 200, 450, 600)

    def okbuttonclicked_2(self):
        reply = QMessageBox.question(self,
                                     "Disconnect to Server",
                                     "确定断开服务器连接吗？",
                                     QMessageBox.Yes | QMessageBox.No,
                                     QMessageBox.Yes)
        if reply == QMessageBox.Yes:
            self.success_disconnection()

    # 关闭
    def cancelbuttonclicked(self):
        self.close()

    def success_connection(self):
        reply = QMessageBox.question(self,
                                     "Connection Success",
                                     "服务器连接成功！",
                                     QMessageBox.Yes,
                                     QMessageBox.Yes)

    def success_disconnection(self):
        reply = QMessageBox.question(self,
                                     "Disconnection Success",
                                     "服务器断开成功！",
                                     QMessageBox.Yes,
                                     QMessageBox.Yes)


class SocketThread(QThread):
    trigger = pyqtSignal()

    def __init__(self, parent=None):
        super(SocketThread, self).__init__(parent)
        self.connect_socket = None

    def run(self):
        global result_info
        global ip_addr
        self.connect_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connect_socket.connect(('127.0.0.1', 8089))
        self.connect_socket.send(b'start')
        self.connect_socket.send(bytes(ip_addr, encoding='utf-8'))
        message = self.connect_socket.recv(1024, 0)
        print(message.decode('utf-8'), len(message.decode('utf-8')))
        result = ''
        while True:
            try:
                info = self.connect_socket.recv(1024, 0)
                if info.decode('utf-8') == '':
                    break
                else:
                    result = result + info.decode('utf-8')
                    print(info.decode('utf-8'))
            except Exception as e:
                print("receive error! " + e)
        result_info = result
        self.trigger.emit()  # 完成后发送完成信号


def okbuttonclicked_1():
    global ip_addr
    ip_addr = scannerWindow.file_path_edit.text()
    scannerWindow.result.setText('扫描中...\n')
    scannerWindow.success_connection()
    mythread.start()
    mythread.trigger.connect(display)


def display():
    global result_info
    print("display"+result_info)
    scannerWindow.result.setText(result_info+"\n")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    scannerWindow = ScannerWindow()
    mythread = SocketThread(scannerWindow)
    scannerWindow.show()
    sys.exit(app.exec_())
