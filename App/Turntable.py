import serial
import time


class Turntable:
    """
    Control a turntable over serial port.
    """

    def __init__(self, port, ackTimeout=0.2):
        """
        Initialize a turntable controller.
        :param port: serial port name
        :type port: str
        :param ackTimeout: how long (seconds) to wait for an acknowledgement
        :type ackTimeout: float
        """
        self.ackTimeout = ackTimeout
        self.serial = serial.Serial(
            port=port, baudrate=115200, timeout=ackTimeout, writeTimeout=ackTimeout)
        for i in range(10):
            self.serial.read()

    def close(self):
        self._cmd("E", immediate=True)
        self.serial.close()

    def _cmd(self, cmd, immediate=False, timeout=None):
        """
        Send a command and wait for completion.
        :param cmd: command string including parameters
        :type cmd: str
        :param immediate: whether the command would complete immediately (no "-" return)
        :type immediate: boolean
        :param timeout: timeout (seconds) for command completion
        :type timeout: float
        :return: whether success
        :rtype: bool
        """
        cmd = ("%s\n" % cmd).encode("ascii")
        self.serial.write(cmd)
        time.sleep(0.1)
        ack = self.serial.read()
        if ack != cmd[0:1]:
            return False
        if immediate:
            return True

        timeout = timeout if timeout is not None else 30.0
        elapsed = 0.0
        while elapsed < timeout:
            ack = self.serial.read()
            if ack == b'-':
                return True
            elapsed = elapsed + self.ackTimeout
        return False

    def zero(self):
        """
        Mark current position as zero.
        """
        return self._cmd("Z", immediate=True)

    def rotateBy(self, n, timeout=None):
        """
        Rotate n degrees from current position.
        :param n: relative position (degrees).
        :type n: float
        """
        return self._cmd("B%0.2f" % n, timeout=timeout)

    def rotateTo(self, n, timeout=None):
        """
        Rotate n degrees from zero position.
        :param n: absolute position (degrees).
        :type n: float
        """
        return self._cmd("T%0.2f" % n, timeout=timeout)

    def stop(self, timeout=None):
        """
        Stop rotation.
        """
        return self._cmd("S", timeout=timeout)

    def estop(self):
        """
        Emergency stop.
        """
        return self._cmd("E", immediate=True)
