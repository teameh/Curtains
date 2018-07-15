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
        self.sendRequest()
    }

    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        view.endEditing(true)
        self.sendRequest()
        return true;
    }

    func textField(_ textField: UITextField, shouldChangeCharactersIn range: NSRange, replacementString string: String) -> Bool {
        let cs = NSCharacterSet(charactersIn: "0123456789").inverted
        let filtered = string.components(separatedBy: cs).joined(separator: "")
        return (string == filtered)
    }

    @IBAction func onChangeValue(_ sender: Any) {
        self.sendRequest()

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
        self.onLoading(status: "Saving...")

        var url = "http://192.168.2.22/settings/\(stepsInput.text ?? "0")/"
        url += "\(speedInput.text ?? "0")/"
        url += "\(accelerationInput.text ?? "0")/"

        Alamofire.request(url)
            .validate()
            .responseJSON { response in
                print("request \(url) \(response.result.isSuccess ? "success" : "failure")");

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
