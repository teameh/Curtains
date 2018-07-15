//
//  ActionsController.swift
//  CurtainsiOS
//
//  Created by Tieme van Veen on 15/07/2018.
//  Copyright © 2018 Tieme van Veen. All rights reserved.
//

import UIKit
import Alamofire

class ActionsController: UIViewController {
    
    @IBOutlet weak var retryButton: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    @IBAction func onOpenFull(_ sender: Any) {
        self.sendOpenRequest(steps: .full, onRetry: #selector(onOpenFull(_:)))
    }
    
    @IBAction func onOpen34(_ sender: Any) {
        self.sendOpenRequest(steps: .threeQuarters, onRetry: #selector(onOpen34(_:)))
    }
    
    @IBAction func onOpen12(_ sender: Any) {
        self.sendOpenRequest(steps: .half, onRetry: #selector(onOpen12(_:)))
    }
    
    @IBAction func onOpen14(_ sender: Any) {
        self.sendOpenRequest(steps: .quarter, onRetry: #selector(onOpen14(_:)))
    }
    
    @IBAction func onStop(_ sender: Any) {
        let url = "http://192.168.2.22/stop/";
        Alamofire.request(url)
            .validate()
            .responseJSON { response in
                print("request \(url) \(response.result.isSuccess ? "success" : "failure")");
                
                switch response.result {
                case .success:
                    self.retryButton.setTitle("Stopping...", for: .normal)
                    DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .seconds(10)) { self.disableRetryButton() }
                case .failure:
                    self.enableButton(title: "Stopping failed... Retry?", action: #selector(self.onStop(_:)))
                }
        }
    }
    
    func sendOpenRequest(steps: Steps, onRetry: Selector) {
        let url = "http://192.168.2.22/open/\(steps.rawValue)/";
        Alamofire.request(url)
            .validate()
            .responseJSON { response in
                print("request \(url) \(response.result.isSuccess ? "success" : "failure")");
                
                switch response.result {
                case .success:
                    self.retryButton.setTitle("Opening...", for: .normal)
                    DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .seconds(10)) { self.disableRetryButton() }
                    self.retryButton.setTitle("Opening...", for: .normal)
                case .failure:
                    self.enableButton(title: "Opening failed... Retry?", action: onRetry)
                }
        }
    }
    
    func disableRetryButton() {
        self.retryButton.isEnabled = false
        self.retryButton.alpha = 0
    }
    
    func enableButton(title: String, action: Selector) {
        self.retryButton.setTitle(title, for: .normal)
        self.retryButton.addTarget(self, action: action, for: .touchUpInside)
    }
}
