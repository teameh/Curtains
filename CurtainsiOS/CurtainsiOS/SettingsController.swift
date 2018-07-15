//
//  SettingsController.swift
//  CurtainsiOS
//
//  Created by Tieme van Veen on 15/07/2018.
//  Copyright © 2018 Tieme van Veen. All rights reserved.
//

import UIKit
import Alamofire

class SettingsController: RequestController, UITextFieldDelegate {

    @IBOutlet weak var stepsInput: UITextField!
    @IBOutlet weak var speedInput: UITextField!
    @IBOutlet weak var accelerationInput: UITextField!

    var throttleTimer: Timer?

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        self.retrieveValues()
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        view.addGestureRecognizer(UITapGestureRecognizer(target: self, action: #selector(dismissKeyboard)))
    }
    
    @objc func dismissKeyboard() {
        view.endEditing(true)
    }

    func textFieldDidEndEditing(_ textField: UITextField) {
        self.sendRequestThrottled()
    }

    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        view.endEditing(true)
        self.sendRequestThrottled()
        return true;
    }

    func textField(_ textField: UITextField, shouldChangeCharactersIn range: NSRange, replacementString string: String) -> Bool {
        let cs = NSCharacterSet(charactersIn: "0123456789").inverted
        let filtered = string.components(separatedBy: cs).joined(separator: "")
        return (string == filtered)
    }

    func sendRequestThrottled() {
        throttleTimer?.invalidate()
        throttleTimer = Timer.scheduledTimer(
            timeInterval: 0.5,
            target: self,
            selector: #selector(sendRequest),
            userInfo: nil,
            repeats: false
        );
    }

    @objc func sendRequest() {
        guard let stepsStr = stepsInput.text, let speedStr = speedInput.text, let accelerationStr = accelerationInput.text else {
            self.onLoading(status: "Invalid value, all inputs must be set")
            return
        }

        guard let steps = Int(stepsStr), steps > 0 else {
            self.onLoading(status: "Steps must be > 0")
            return
        }

        guard let speed = Int(speedStr), steps > 0 else {
            self.onLoading(status: "Speed must be > 0")
            return
        }

        guard let acceleration = Int(accelerationStr), acceleration > 0 else {
            self.onLoading(status: "Acceleration must be > 0")
            return
        }

        self.onLoading(status: "Saving...")

        let url = "http://192.168.2.22/settings"
        let parameters: Parameters = [
            "steps": steps,
            "speed": speed,
            "acceleration": acceleration,
        ]

        Alamofire.request(url, parameters: parameters)
            .validate()
            .responseJSON { response in
                print("request \(response.response!.statusCode) - \(response.request!.url!) \(String(describing: response.result.value))");

                switch response.result {
                case .success:
                    self.onSuccess(status: "Saved")
                    self.storeValues()
                case .failure(let err):
                    self.onFailure(status: "Request failed... \(err.localizedDescription)", action: #selector(self.sendRequest))
                }
        }
    }
    
    func storeValues() {
        UserDefaults.standard.set(self.stepsInput.text, forKey: "settings.steps")
        UserDefaults.standard.set(self.speedInput.text, forKey: "settings.speed")
        UserDefaults.standard.set(self.accelerationInput.text, forKey: "settings.acceleration")
    }

    func retrieveValues() {
        if let value = UserDefaults.standard.string(forKey: "settings.steps") {
            self.stepsInput.text = value
        }
        if let value = UserDefaults.standard.string(forKey: "settings.speed") {
            self.speedInput.text = value
        }
        if let value = UserDefaults.standard.string(forKey: "settings.acceleration") {
            self.accelerationInput.text = value
        }
    }
}
